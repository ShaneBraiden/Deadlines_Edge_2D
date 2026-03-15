#include "Remnant.h"
#include "Utility.h"
#include "Constants.h"
#include <cmath>

Remnant::Remnant(PhysicsWorld* physics, float worldX, float worldY)
    : physics(physics)
    , currentRotation(0.0f)
    , targetRotation(0.0f)
    , worldX(worldX)
    , worldY(worldY)
{
    // Create a static Box2D body (Remnants don't move)
    float halfW = 0.4f;
    float halfH = 0.9f;
    body = physics->createStaticBody(worldX, worldY, halfW, halfH);
    body->GetUserData().pointer = reinterpret_cast<uintptr_t>(this);

    // Build a dark silhouette shape (tall, slightly humanoid)
    float pixelW = halfW * 2.0f * Constants::PPM;
    float pixelH = halfH * 2.0f * Constants::PPM;

    // Simple rectangular silhouette with slightly tapered top
    shape.setPointCount(6);
    shape.setPoint(0, sf::Vector2f(-pixelW * 0.5f, pixelH * 0.5f));   // bottom-left
    shape.setPoint(1, sf::Vector2f( pixelW * 0.5f, pixelH * 0.5f));   // bottom-right
    shape.setPoint(2, sf::Vector2f( pixelW * 0.45f, -pixelH * 0.1f)); // right shoulder
    shape.setPoint(3, sf::Vector2f( pixelW * 0.25f, -pixelH * 0.5f)); // top-right
    shape.setPoint(4, sf::Vector2f(-pixelW * 0.25f, -pixelH * 0.5f)); // top-left
    shape.setPoint(5, sf::Vector2f(-pixelW * 0.45f, -pixelH * 0.1f)); // left shoulder

    shape.setFillColor(sf::Color(8, 8, 10, 200));       // Near-black, slightly transparent
    shape.setOutlineColor(sf::Color(20, 20, 25, 150));
    shape.setOutlineThickness(1.0f);

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
    }
}

void Remnant::render(sf::RenderWindow& window) {
    sf::Vector2f screenPos = physics->toScreen(b2Vec2(worldX, worldY));
    shape.setPosition(screenPos);

    // Apply subtle rotation based on player proximity
    // Only rotate the visual slightly (not the full angle, just a lean)
    float leanAngle = Utility::clamp(currentRotation * 0.05f, -15.0f, 15.0f);
    shape.setRotation(leanAngle);

    // Proximity-based alpha — more visible when player is closer
    float distance = getDistanceToPlayer();
    if (distance < DETECTION_RANGE) {
        float proximityFactor = 1.0f - (distance / DETECTION_RANGE);
        int alpha = static_cast<int>(Utility::lerp(100.0f, 220.0f, proximityFactor));
        shape.setFillColor(sf::Color(8, 8, 10, alpha));
    }
    else {
        shape.setFillColor(sf::Color(8, 8, 10, 80));  // Barely visible at distance
    }

    window.draw(shape);
}
