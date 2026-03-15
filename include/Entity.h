#pragma once

#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>

// Base class for all game entities (Player, Remnants, etc.).
//
// WHY a base class:
//   Enables polymorphism — Game can store Entity* pointers and call
//   update()/render() without knowing the concrete type. This directly
//   covers the lab requirements for single-level pointers and the
//   arrow operator (entity->update(), entity->render()).
//
// Lab requirements covered:
//   - Single-level pointer: b2Body* body
//   - Arrow operator: body->GetPosition(), body->GetLinearVelocity()

class Entity {
public:
    Entity() = default;
    virtual ~Entity() = default;

    // Every entity must define how it updates and renders.
    virtual void update(float dt) = 0;
    virtual void render(sf::RenderWindow& window) = 0;

    // Ground contact callbacks — called by ContactListener when the
    // entity's foot sensor touches or leaves a surface.
    // Default: no-op. Override in subclasses that need ground detection.
    virtual void beginGroundContact() {}
    virtual void endGroundContact() {}

    // Wall contact callbacks
    virtual void beginWallContact(bool leftSide) {}
    virtual void endWallContact(bool leftSide) {}

    // Lab: single-level pointer access + arrow operator
    b2Body* getBody() const { return body; }

protected:
    b2Body* body = nullptr;   // Lab: single-level pointer
                              // Owned by b2World, NOT by this class
};
