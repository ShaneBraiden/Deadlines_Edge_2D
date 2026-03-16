#pragma once

#include "Entity.h"
#include "PhysicsWorld.h"
#include "Animation.h"

// Remnant — the silent watchers. Core horror element.
//
// Remnants are dark silhouettes that slowly rotate to face the player
// when the player is nearby. They don't move or attack — they just watch.
//
// Animation:
//   "idle" — subtle sway (6 frames)
//   "alert" — turn toward player when detected (4 frames)
//
// Lab requirements covered:
//   - Single-level pointer: PhysicsWorld*, b2Body* (inherited)
//   - Arrow operator: physics->toScreen(), body->GetPosition()
//   - STL map: AnimationPlayer uses std::map<std::string, Animation>

class Remnant : public Entity {
public:
    Remnant(PhysicsWorld* physics, float worldX, float worldY, sf::Texture& spritesheet);

    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    // Set the target position the Remnant slowly looks toward.
    void setPlayerPosition(const sf::Vector2f& pos);

    // Get distance to player in meters.
    float getDistanceToPlayer() const;

    // Detection range in meters.
    static constexpr float DETECTION_RANGE = 8.0f;

private:
    PhysicsWorld* physics;          // Lab: single-level pointer
    sf::Sprite sprite;              // Animated sprite (replaces ConvexShape)
    AnimationPlayer animator;       // Drives sway/alert animations
    sf::Vector2i frameSize;         // Pixel dimensions of one spritesheet frame
    sf::Vector2f playerScreenPos;   // Cached player screen position
    float currentRotation;          // Current facing angle in degrees
    float targetRotation;           // Where we want to face
    float worldX, worldY;           // Position in world coords (meters)
    bool alerting;                  // Currently in alert state
};
