#pragma once

#include "Entity.h"
#include "PhysicsWorld.h"
#include "InputManager.h"

// Ethan Calloway — the player character.
//
// Movement design:
//   Horizontal: velocity interpolation (lerp toward desired speed).
//   Vertical: Box2D gravity + jump impulse.
//   Wall jump: jump off walls when touching and pressing jump.
//   Sprint: hold shift for faster movement.
//
// Lab requirements covered:
//   - Single-level pointer: PhysicsWorld*, inherited b2Body*
//   - Arrow operator: physics->toScreen(), body->GetLinearVelocity()

class Player : public Entity {
public:
    Player(PhysicsWorld* physics, float startX, float startY);

    void handleInput(const InputManager& input);
    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    // Contact tracking (called by ContactListener)
    void beginGroundContact() override;
    void endGroundContact() override;

    // Wall contact tracking
    void beginWallContact(bool leftSide);
    void endWallContact(bool leftSide);

    bool isOnGround() const;
    bool isTouchingWall() const;

private:
    PhysicsWorld* physics;             // Lab: single-level pointer, arrow operator
    sf::RectangleShape shape;          // Visual placeholder for Ethan
    int  groundContactCount;           // >0 means standing on something
    int  leftWallContactCount;         // >0 means touching left wall
    int  rightWallContactCount;        // >0 means touching right wall
    bool sprinting;                    // Currently sprinting
};
