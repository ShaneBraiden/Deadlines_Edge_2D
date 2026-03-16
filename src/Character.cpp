#include "Character.h"
#include "Constants.h"
#include <cmath>

Character::Character(PhysicsWorld* physics, sf::Texture& texture,
                     sf::Vector2i frameSize, float startX, float startY)
    : physics(physics)
    , frameSize(frameSize)
    , animState(AnimState::IDLE)
    , facingRight(true)
    , groundContactCount(0)
{
    // --- Box2D body ---
    // Size the physics body to match the sprite's pixel dimensions in meters.
    float halfW = (frameSize.x / Constants::PPM) / 2.0f;
    float halfH = (frameSize.y / Constants::PPM) / 2.0f;

    body = physics->createDynamicBody(
        startX, startY, halfW, halfH,
        Constants::PLAYER_DENSITY, Constants::PLAYER_FRICTION
    );
    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    // Foot sensor for ground detection
    b2Vec2 footOffset(0.0f, -halfH);
    physics->addSensorFixture(body, halfW - 0.05f, 0.1f, footOffset,
                              Constants::FIXTURE_FOOT_SENSOR);

    // --- SFML sprite ---
    sprite.setTexture(texture);
    sprite.setOrigin(frameSize.x / 2.0f, frameSize.y / 2.0f);

    // --- Animations (rows 0-3 of the spritesheet) ---
    // Adjust frame counts and durations to match your actual spritesheet.
    float frameDur = 0.1f;
    animator.addAnimation("idle", Animation(0, 6, frameSize, 0.15f));
    animator.addAnimation("run",  Animation(1, 8, frameSize, 0.08f));
    animator.addAnimation("jump", Animation(2, 3, frameSize, frameDur));
    animator.addAnimation("fall", Animation(3, 3, frameSize, frameDur));

    animator.play("idle");
}

void Character::update(float dt) {
    updateAnimState();
    animator.update(dt, sprite);
    syncSpriteToBody();
}

void Character::render(sf::RenderWindow& window) {
    window.draw(sprite);
}

// --- Contact callbacks ---
void Character::beginGroundContact() { groundContactCount++; }
void Character::endGroundContact()   { groundContactCount--; }
bool Character::isOnGround() const   { return groundContactCount > 0; }

AnimState Character::getAnimState() const { return animState; }

// Determine animation state from Box2D velocity + ground contact.
void Character::updateAnimState() {
    b2Vec2 vel = body->GetLinearVelocity();
    AnimState newState = animState;

    if (!isOnGround()) {
        // Box2D Y is up: positive = rising, negative = falling
        newState = (vel.y > 0.5f) ? AnimState::JUMP : AnimState::FALL;
    } else {
        newState = (std::abs(vel.x) > 0.5f) ? AnimState::RUN : AnimState::IDLE;
    }

    // Flip sprite based on horizontal movement direction
    if (vel.x > 0.1f)       facingRight = true;
    else if (vel.x < -0.1f) facingRight = false;

    // Apply scale to flip horizontally (preserves vertical scale)
    float scaleX = facingRight ? 1.0f : -1.0f;
    sprite.setScale(scaleX, 1.0f);

    // Switch animation if state changed
    if (newState != animState) {
        animState = newState;
        switch (animState) {
            case AnimState::IDLE: animator.play("idle");          break;
            case AnimState::RUN:  animator.play("run");           break;
            case AnimState::JUMP: animator.play("jump", false);   break;
            case AnimState::FALL: animator.play("fall");          break;
        }
    }
}

// Convert Box2D body position to SFML screen coordinates.
void Character::syncSpriteToBody() {
    b2Vec2 bodyPos = body->GetPosition();
    sf::Vector2f screenPos = physics->toScreen(bodyPos);
    sprite.setPosition(screenPos);
}
