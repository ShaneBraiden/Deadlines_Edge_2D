#pragma once

#include "Entity.h"
#include "PhysicsWorld.h"

// Remnant — the silent watchers. Core horror element.
//
// Remnants are dark silhouettes that slowly rotate to face the player
// when the player is nearby. They don't move or attack — they just watch.
//
// Lab requirements covered:
//   - Single-level pointer: PhysicsWorld*, b2Body* (inherited)
//   - Arrow operator: physics->toScreen(), body->GetPosition()

class Remnant : public Entity {
public:
    Remnant(PhysicsWorld* physics, float worldX, float worldY);

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
    sf::ConvexShape shape;          // Dark silhouette shape
    sf::Vector2f playerScreenPos;   // Cached player screen position
    float currentRotation;          // Current facing angle in degrees
    float targetRotation;           // Where we want to face
    float worldX, worldY;           // Position in world coords (meters)
};
