#include "Player.h"
#include <cmath>

Player::Player(PhysicsWorld* physics, float startX, float startY, sf::Texture& spritesheet)
    : physics(physics)
    , frameSize(128, 128)
    , animState(AnimState::IDLE)
    , facingRight(true)
    , groundContactCount(0)
    , leftWallContactCount(0)
    , rightWallContactCount(0)
    , sprinting(false)
    , runnerModeEnabled(false)
    , ducking(false)
    , invulnerabilityTimer(0.0f)
    , hitReactionTimer(0.0f)
    , flashTimer(0.0f)
    , flashVisible(true)
{
    float halfW = Constants::PLAYER_WIDTH / 2.0f;
    float halfH = Constants::PLAYER_HEIGHT / 2.0f;

    // Create dynamic body in Box2D
    body = physics->createDynamicBody(               // Lab: arrow operator
        startX, startY,
        halfW, halfH,
        Constants::PLAYER_DENSITY,
        Constants::PLAYER_FRICTION
    );

    // Store 'this' pointer in body user data so ContactListener can reach us
    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);   // Lab: pointer cast

    // Foot sensor — extends BELOW the player body to detect ground
    b2Vec2 footOffset(0.0f, -halfH - 0.15f);          // Slightly below bottom edge
    physics->addSensorFixture(                        // Lab: arrow operator
        body,
        halfW * 0.8f,      // sensor half-width (slightly narrower than body)
        0.2f,              // sensor half-height (thicker for reliable detection)
        footOffset,
        Constants::FIXTURE_FOOT_SENSOR
    );

    // Left wall sensor — thin box on the left side of the player
    b2Vec2 leftOffset(-halfW, 0.0f);
    physics->addSensorFixture(
        body,
        0.1f,              // sensor half-width (thin)
        halfH - 0.1f,      // sensor half-height (slightly shorter than body)
        leftOffset,
        Constants::FIXTURE_LEFT_WALL_SENSOR
    );

    // Right wall sensor — thin box on the right side of the player
    b2Vec2 rightOffset(halfW, 0.0f);
    physics->addSensorFixture(
        body,
        0.1f,
        halfH - 0.1f,
        rightOffset,
        Constants::FIXTURE_RIGHT_WALL_SENSOR
    );

    // SFML sprite setup
    sprite.setTexture(spritesheet);
    sprite.setOrigin(frameSize.x / 2.0f, frameSize.y / 2.0f);

    // Register animations (row, frameCount, frameSize, frameDuration)
    animator.addAnimation("idle", Animation(0, 6, frameSize, 0.15f));
    animator.addAnimation("run",  Animation(1, 8, frameSize, 0.08f));
    animator.addAnimation("jump", Animation(2, 3, frameSize, 0.12f));
    animator.addAnimation("fall", Animation(3, 3, frameSize, 0.10f));
    // Hit and stumble reuse existing frames for now
    animator.addAnimation("hit",  Animation(3, 2, frameSize, 0.15f));
    animator.addAnimation("stumble", Animation(1, 4, frameSize, 0.10f));
    animator.play("idle");
}

void Player::setRunnerMode(bool enabled) {
    runnerModeEnabled = enabled;
}

void Player::handleInput(const InputManager& input) {
    // --- Sprint ---
    sprinting = !runnerModeEnabled && input.isKeyHeld(sf::Keyboard::LShift);

    // --- Horizontal movement ---
    float desiredVelX = runnerModeEnabled ? Constants::PLAYER_MOVE_SPEED : 0.0f;
    float moveSpeed = Constants::PLAYER_MOVE_SPEED;
    if (sprinting) {
        moveSpeed *= Constants::PLAYER_SPRINT_MULTIPLIER;
    }

    if (!runnerModeEnabled && (input.isKeyHeld(sf::Keyboard::A) ||
        input.isKeyHeld(sf::Keyboard::Left))) {
        desiredVelX -= moveSpeed;
    }
    if (!runnerModeEnabled && (input.isKeyHeld(sf::Keyboard::D) ||
        input.isKeyHeld(sf::Keyboard::Right))) {
        desiredVelX += moveSpeed;
    }

    // Smooth velocity interpolation for the heavy/floaty feel.
    b2Vec2 vel = body->GetLinearVelocity();                    // Lab: arrow operator
    float accel = isOnGround() ? Constants::PLAYER_GROUND_ACCEL
                               : Constants::PLAYER_AIR_ACCEL;
    vel.x += (desiredVelX - vel.x) * accel;
    body->SetLinearVelocity(vel);                              // Lab: arrow operator

    // --- Jump / Wall Jump ---
    bool jumpKeyDown = input.isKeyPressed(sf::Keyboard::Space) ||
                       input.isKeyPressed(sf::Keyboard::W) ||
                       input.isKeyPressed(sf::Keyboard::Up);
    bool isDoubleClick = input.isSpaceBarDoubleClicked();

    if (jumpKeyDown || isDoubleClick) {
        // Allow jump if on ground OR if vertical velocity is very low (coyote-time-like fallback)
        b2Vec2 currentVel = body->GetLinearVelocity();
        bool canJump = isOnGround() || (std::abs(currentVel.y) < 0.5f);

        if (canJump) {
            // Double-click gives a higher jump (+50%)
            float jumpMultiplier = isDoubleClick ? 1.5f : 1.0f;
            float impulse = Constants::PLAYER_JUMP_IMPULSE * jumpMultiplier * body->GetMass();
            body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, impulse), true);
        }
        else if (!runnerModeEnabled && isTouchingWall()) {
            // Wall jump — push away from wall and upward
            float horizontalDir = (leftWallContactCount > 0) ? 1.0f : -1.0f;
            float mass = body->GetMass();
            float hImpulse = Constants::WALL_JUMP_HORIZONTAL * mass * horizontalDir;
            float vImpulse = Constants::WALL_JUMP_VERTICAL * mass;

            // Reset velocity before applying wall jump impulse
            body->SetLinearVelocity(b2Vec2(0.0f, 0.0f));
            body->ApplyLinearImpulseToCenter(b2Vec2(hImpulse, vImpulse), true);
        }
    }

    // --- Duck / Fast-fall ---
    bool duckKeyHeld = input.isKeyHeld(sf::Keyboard::S) ||
                       input.isKeyHeld(sf::Keyboard::Down);
    ducking = duckKeyHeld && isOnGround();

    // Fast-fall when pressing down in the air
    if (duckKeyHeld && !isOnGround()) {
        b2Vec2 currentVel = body->GetLinearVelocity();
        if (currentVel.y > -15.0f) {
            body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, -8.0f * body->GetMass()), true);
        }
    }
}

void Player::update(float dt) {
    // Update invulnerability
    if (invulnerabilityTimer > 0.0f) {
        invulnerabilityTimer -= dt;

        // Flash effect
        flashTimer += dt;
        if (flashTimer >= 0.08f) {
            flashTimer = 0.0f;
            flashVisible = !flashVisible;
        }
    } else {
        flashVisible = true;
    }

    // Update hit reaction
    if (hitReactionTimer > 0.0f) {
        hitReactionTimer -= dt;
        if (hitReactionTimer <= 0.0f && animState == AnimState::HIT) {
            animState = AnimState::STUMBLE;
            animator.play("stumble", false);
        }
    }

    updateAnimState();
    animator.update(dt, sprite);
    syncSpriteToBody();
}

void Player::render(sf::RenderWindow& window) {
    // Don't render if flashing and invisible
    if (!flashVisible && invulnerabilityTimer > 0.0f) {
        return;
    }

    // Tint red if hit reacting
    if (hitReactionTimer > 0.0f) {
        sprite.setColor(sf::Color(255, 150, 150));
    } else if (invulnerabilityTimer > 0.0f) {
        sprite.setColor(sf::Color(255, 255, 255, 180));
    } else {
        sprite.setColor(sf::Color::White);
    }

    window.draw(sprite);
}

sf::FloatRect Player::getBoundsScreen() const {
    sf::FloatRect bounds = sprite.getGlobalBounds();
    // If ducking, shrink the hitbox height
    if (ducking) {
        float reduction = bounds.height * 0.45f;
        bounds.top += reduction;
        bounds.height -= reduction;
    }
    return bounds;
}

bool Player::isDucking() const {
    return ducking;
}

void Player::updateAnimState() {
    // Skip animation updates during hit reaction
    if (animState == AnimState::HIT) {
        return;
    }
    if (animState == AnimState::STUMBLE && !animator.isFinished()) {
        return;
    }

    b2Vec2 vel = body->GetLinearVelocity();
    AnimState newState = animState;

    if (!isOnGround()) {
        // Box2D Y is up: positive = rising, negative = falling
        newState = (vel.y > 0.5f) ? AnimState::JUMP : AnimState::FALL;
    } else if (ducking) {
        newState = AnimState::DUCK;
    } else {
        newState = (runnerModeEnabled || std::abs(vel.x) > 0.5f) ? AnimState::RUN : AnimState::IDLE;
    }

    // Flip sprite based on horizontal movement direction
    if (vel.x > 0.1f)       facingRight = true;
    else if (vel.x < -0.1f) facingRight = false;

    // Scale sprite so its height matches the physics body's pixel height
    float physPixelH = Constants::PLAYER_HEIGHT * Constants::PPM;
    float scaleY = physPixelH / static_cast<float>(frameSize.y);
    // Squash vertically when ducking
    if (ducking) {
        scaleY *= 0.55f;
    }
    float scaleX = facingRight ? scaleY : -scaleY;
    sprite.setScale(scaleX, scaleY);

    // Switch animation if state changed
    if (newState != animState) {
        animState = newState;
        switch (animState) {
            case AnimState::IDLE:    animator.play("idle");          break;
            case AnimState::RUN:     animator.play("run");           break;
            case AnimState::JUMP:    animator.play("jump", false);   break;
            case AnimState::FALL:    animator.play("fall");          break;
            case AnimState::DUCK:    animator.play("run");           break;  // Use run frames for duck
            case AnimState::HIT:     animator.play("hit", false);    break;
            case AnimState::STUMBLE: animator.play("stumble", false); break;
        }
    }
}

void Player::syncSpriteToBody() {
    b2Vec2 bodyPos = body->GetPosition();                   // Lab: arrow operator
    sf::Vector2f screenPos = physics->toScreen(bodyPos);    // Lab: arrow operator
    sprite.setPosition(screenPos);
}

void Player::beginGroundContact() {
    groundContactCount++;
}

void Player::endGroundContact() {
    groundContactCount--;
}

void Player::beginWallContact(bool leftSide) {
    if (leftSide) leftWallContactCount++;
    else rightWallContactCount++;
}

void Player::endWallContact(bool leftSide) {
    if (leftSide) leftWallContactCount--;
    else rightWallContactCount--;
}

bool Player::isOnGround() const {
    return groundContactCount > 0;
}

bool Player::isTouchingWall() const {
    return leftWallContactCount > 0 || rightWallContactCount > 0;
}

void Player::hit() {
    if (invulnerabilityTimer > 0.0f) return;  // Already invulnerable

    animState = AnimState::HIT;
    animator.play("hit", false);
    hitReactionTimer = 0.3f;
    invulnerabilityTimer = Constants::PLAYER_INVULNERABILITY_TIME;
    flashTimer = 0.0f;
    flashVisible = true;

    // Small knockback
    b2Vec2 vel = body->GetLinearVelocity();
    vel.y = 5.0f;  // Small upward bounce
    body->SetLinearVelocity(vel);
}

void Player::setInvulnerable(float duration) {
    invulnerabilityTimer = duration;
    flashTimer = 0.0f;
}

sf::Vector2f Player::getScreenPosition() const {
    b2Vec2 bodyPos = body->GetPosition();
    return physics->toScreen(bodyPos);
}
