#pragma once

#include "Entity.h"
#include "PhysicsWorld.h"
#include "Animation.h"

// Animation states driven by Box2D velocity.
enum class AnimState {
    IDLE,
    RUN,
    JUMP,
    FALL
};

// An animated character entity: b2Body + sf::Sprite + AnimationPlayer.
// Automatically selects animation based on physics velocity and
// flips the sprite to face movement direction.
class Character : public Entity {
public:
    // physics     = world for coordinate conversion
    // texture     = the spritesheet (must outlive this object)
    // frameSize   = pixel size of one frame (e.g. {48,48})
    // startX/Y    = spawn position in Box2D meters
    Character(PhysicsWorld* physics, sf::Texture& texture,
              sf::Vector2i frameSize, float startX, float startY);

    void update(float dt) override;
    void render(sf::RenderWindow& window) override;

    void beginGroundContact() override;
    void endGroundContact() override;

    bool isOnGround() const;
    AnimState getAnimState() const;

private:
    void updateAnimState();
    void syncSpriteToBody();

    PhysicsWorld* physics;
    sf::Sprite sprite;
    AnimationPlayer animator;
    sf::Vector2i frameSize;

    AnimState animState;
    bool facingRight;
    int groundContactCount;
};
