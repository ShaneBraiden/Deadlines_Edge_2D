#include "Remnant.h"
#include "Utility.h"
#include "Constants.h"
#include <cmath>

Remnant::Remnant(PhysicsWorld* physics, float worldX, float worldY, sf::Texture& spritesheet)
    : physics(physics)
    , frameSize(128, 128)
    , currentRotation(0.0f)
    , targetRotation(0.0f)
    , worldX(worldX)
    , worldY(worldY)
    , alerting(false)
{
    // Create a static Box2D body (Remnants don't move)
    float halfW = 0.4f;
    float halfH = 0.9f;
    body = physics->createStaticBody(worldX, worldY, halfW, halfH);
    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    // SFML sprite setup
    sprite.setTexture(spritesheet);
    sprite.setOrigin(frameSize.x / 2.0f, frameSize.y / 2.0f);

    // Scale sprite to match physics body height (1.8m = 90px at PPM=50)
    float physPixelH = halfH * 2.0f * Constants::PPM;
    float scale = physPixelH / static_cast<float>(frameSize.y);
    sprite.setScale(scale, scale);

    // Register animations (row, frameCount, frameSize, frameDuration)
    animator.addAnimation("idle",  Animation(0, 6, frameSize, 0.20f));
    animator.addAnimation("alert", Animation(1, 4, frameSize, 0.15f));
    animator.play("idle");

    playerScreenPos = sf::Vector2f(0.0f, 0.0f);
}

void Remnant::setPlayerPosition(const sf::Vector2f& pos) {
    playerScreenPos = pos;
}

float Remnant::getDistanceToPlayer() const {
    sf::Vector2f myScreenPos = physics->toScreen(b2Vec2(worldX, worldY));
    float dx = playerScreenPos.x - myScreenPos.x;
    float dy = playerScreenPos.y - myScreenPos.y;
    // Convert pixel distance to meters
    float pixelDist = std::sqrt(dx * dx + dy * dy);
    return pixelDist / Constants::PPM;
}

void Remnant::update(float dt) {
    float distance = getDistanceToPlayer();

    if (distance < DETECTION_RANGE) {
        // Calculate angle to player
        sf::Vector2f myScreenPos = physics->toScreen(b2Vec2(worldX, worldY));
        float dx = playerScreenPos.x - myScreenPos.x;
        float dy = playerScreenPos.y - myScreenPos.y;
        targetRotation = std::atan2(dy, dx) * 180.0f / 3.14159265f;

        // Slow rotation toward player -- Lab: function template (lerp<T>)
        // Closer player = faster rotation (creepier)
        float proximityFactor = 1.0f - (distance / DETECTION_RANGE);
        float rotSpeed = Utility::lerp(0.5f, 3.0f, proximityFactor);

        // Smooth interpolation of rotation
        float angleDiff = targetRotation - currentRotation;

        // Normalize angle difference to [-180, 180]
        while (angleDiff > 180.0f) angleDiff -= 360.0f;
        while (angleDiff < -180.0f) angleDiff += 360.0f;

        currentRotation += angleDiff * rotSpeed * dt;

        // Switch to alert animation when player is close enough
        if (!alerting && proximityFactor > 0.3f) {
            alerting = true;
            animator.play("alert", false);
        }
    } else {
        // Return to idle when player moves away
        if (alerting) {
            alerting = false;
            animator.play("idle");
        }
    }

    animator.update(dt, sprite);
}

void Remnant::render(sf::RenderWindow& window) {
    sf::Vector2f screenPos = physics->toScreen(b2Vec2(worldX, worldY));
    sprite.setPosition(screenPos);

    // Apply subtle rotation based on player proximity
    // Only rotate the visual slightly (not the full angle, just a lean)
    float leanAngle = Utility::clamp(currentRotation * 0.05f, -15.0f, 15.0f);
    sprite.setRotation(leanAngle);

    // Proximity-based alpha — more visible when player is closer
    float distance = getDistanceToPlayer();
    if (distance < DETECTION_RANGE) {
        float proximityFactor = 1.0f - (distance / DETECTION_RANGE);
        int alpha = static_cast<int>(Utility::lerp(100.0f, 220.0f, proximityFactor));
        sprite.setColor(sf::Color(255, 255, 255, alpha));
    }
    else {
        sprite.setColor(sf::Color(255, 255, 255, 80));  // Barely visible at distance
    }

    window.draw(sprite);
}
