#include "Player.h"

Player::Player(PhysicsWorld* physics, float startX, float startY)
    : physics(physics)
    , groundContactCount(0)
    , leftWallContactCount(0)
    , rightWallContactCount(0)
    , sprinting(false)
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

    // Foot sensor — thin box at the bottom of the player.
    b2Vec2 footOffset(0.0f, -halfH);                 // Bottom edge (Y-up)
    physics->addSensorFixture(                        // Lab: arrow operator
        body,
        halfW - 0.05f,     // sensor half-width
        0.1f,              // sensor half-height (thin)
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

    // SFML visual shape
    float pixelW = Constants::PLAYER_WIDTH  * Constants::PPM;
    float pixelH = Constants::PLAYER_HEIGHT * Constants::PPM;
    shape.setSize(sf::Vector2f(pixelW, pixelH));
    shape.setOrigin(pixelW / 2.0f, pixelH / 2.0f);  // Center origin for Box2D alignment
    shape.setFillColor(sf::Color(74, 106, 138));      // Muted blue-gray
    shape.setOutlineColor(sf::Color(90, 130, 170));
    shape.setOutlineThickness(1.0f);
}

void Player::handleInput(const InputManager& input) {
    // --- Sprint ---
    sprinting = input.isKeyHeld(sf::Keyboard::LShift);

    // --- Horizontal movement ---
    float desiredVelX = 0.0f;
    float moveSpeed = Constants::PLAYER_MOVE_SPEED;
    if (sprinting) {
        moveSpeed *= Constants::PLAYER_SPRINT_MULTIPLIER;
    }

    if (input.isKeyHeld(sf::Keyboard::A) ||
        input.isKeyHeld(sf::Keyboard::Left)) {
        desiredVelX -= moveSpeed;
    }
    if (input.isKeyHeld(sf::Keyboard::D) ||
        input.isKeyHeld(sf::Keyboard::Right)) {
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

    if (jumpKeyDown) {
        if (isOnGround()) {
            // Normal jump
            float impulse = Constants::PLAYER_JUMP_IMPULSE * body->GetMass();
            body->ApplyLinearImpulseToCenter(b2Vec2(0.0f, impulse), true);
        }
        else if (isTouchingWall()) {
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

    // Visual feedback: tint slightly when sprinting
    if (sprinting) {
        shape.setFillColor(sf::Color(90, 120, 155));
    } else {
        shape.setFillColor(sf::Color(74, 106, 138));
    }
}

void Player::update(float dt) {
    // Physics handles all position updates via Box2D.
}

void Player::render(sf::RenderWindow& window) {
    // Convert Box2D world position (meters, Y-up) to SFML screen position (pixels, Y-down)
    b2Vec2 bodyPos = body->GetPosition();                   // Lab: arrow operator
    sf::Vector2f screenPos = physics->toScreen(bodyPos);    // Lab: arrow operator

    shape.setPosition(screenPos);
    window.draw(shape);
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
