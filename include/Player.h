#pragma once

#include "Entity.h"
#include "PhysicsWorld.h"
#include "InputManager.h"
#include "Animation.h"

// Animation states driven by Box2D velocity.
enum class AnimState {
    IDLE,
    RUN,
    JUMP,
    FALL
};

// Ethan Calloway — the player character.
//
// Movement design:
//   Horizontal: velocity interpolation (lerp toward desired speed).
//   Vertical: Box2D gravity + jump impulse.
//   Wall jump: jump off walls when touching and pressing jump.
//   Sprint: hold shift for faster movement.
//
// Animation:
//   AnimationPlayer drives sf::Sprite frame selection from a spritesheet.
//   AnimState auto-switches based on Box2D linear velocity + ground contact.
//   Sprite flips horizontally based on movement direction.
//
// Lab requirements covered:
//   - Single-level pointer: PhysicsWorld*, inherited b2Body*
//   - Arrow operator: physics->toScreen(), body->GetLinearVelocity()
//   - STL map: AnimationPlayer uses std::map<std::string, Animation>

class Player : public Entity {
public:
    Player(PhysicsWorld* physics, float startX, float startY, sf::Texture& spritesheet);

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
    void updateAnimState();
    void syncSpriteToBody();

    PhysicsWorld* physics;             // Lab: single-level pointer, arrow operator
    sf::Sprite sprite;                 // Animated sprite (replaces RectangleShape)
    AnimationPlayer animator;          // Drives frame-by-frame animation
    sf::Vector2i frameSize;            // Pixel dimensions of one spritesheet frame
    AnimState animState;               // Current animation state
    bool facingRight;                  // Horizontal flip tracking
    int  groundContactCount;           // >0 means standing on something
    int  leftWallContactCount;         // >0 means touching left wall
    int  rightWallContactCount;        // >0 means touching right wall
    bool sprinting;                    // Currently sprinting
};
