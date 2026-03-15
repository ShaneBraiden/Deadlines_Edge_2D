#pragma once

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "Entity.h"
#include "Constants.h"

// Detects when an entity's foot sensor touches or leaves a surface.
//
// WHY a contact listener:
//   Box2D doesn't expose "is this body on the ground" directly. We attach
//   a thin sensor fixture to the bottom of the player. When it overlaps
//   with any static body (ground, platforms), we increment a contact counter.
//   This lets us know if the player can jump.
//
// Lab requirements covered:
//   - Arrow operator: fixture->GetUserData(), body->GetUserData()
//   - Single-level pointer: b2Fixture*, b2Body*, Entity*

class ContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;

private:
    void handleContact(b2Contact* contact, bool begin);
};


// Wraps Box2D's b2World so the rest of the game doesn't interact
// with Box2D types directly (except through body pointers).
//
// WHY a wrapper:
//   Encapsulation — if we ever swap physics engines, only this class changes.
//   Also centralizes coordinate conversion (Box2D meters ↔ SFML pixels).
//
// Coordinate systems:
//   Box2D: meters, Y points UP,  origin at bottom-left
//   SFML:  pixels, Y points DOWN, origin at top-left
//
// Lab requirements covered:
//   - Single-level pointer: b2World* world
//   - Arrow operator: world->Step(), world->CreateBody()

class PhysicsWorld {
public:
    PhysicsWorld(float windowWidth, float windowHeight);
    ~PhysicsWorld();

    // Advance physics simulation by one fixed timestep
    void step();

    // Create bodies in the physics world
    b2Body* createDynamicBody(float centerX, float centerY,
                              float halfWidth, float halfHeight,
                              float density, float friction,
                              bool fixedRotation = true);

    b2Body* createStaticBody(float centerX, float centerY,
                             float halfWidth, float halfHeight);

    // Attach a sensor fixture to an existing body
    void addSensorFixture(b2Body* body, float halfWidth, float halfHeight,
                          const b2Vec2& offset, uintptr_t userData);

    // --- Coordinate conversion ---
    sf::Vector2f toScreen(const b2Vec2& worldPos) const;
    b2Vec2       toWorld(const sf::Vector2f& screenPos) const;
    float        toPixels(float meters) const;
    float        toMeters(float pixels) const;

    // Accessors
    b2World* getWorld();             // Lab: arrow operator via world->...
    float    getWorldWidth() const;  // World size in meters
    float    getWorldHeight() const;

private:
    b2World* world;                  // Lab: single-level pointer
    ContactListener contactListener;
    float windowWidth;               // Pixels
    float windowHeight;              // Pixels
    float worldWidth;                // Meters
    float worldHeight;               // Meters
};
