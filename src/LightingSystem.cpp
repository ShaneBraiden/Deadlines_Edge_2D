#include "LightingSystem.h"
#include <cmath>

LightingSystem::LightingSystem(unsigned int width, unsigned int height)
    : ambientLevel(25)
    , flickerTimer(0.0f)
{
    lightMap = new sf::RenderTexture();   // Lab: single-level pointer via new
    lightMap->create(width, height);
}

LightingSystem::~LightingSystem() {
    delete lightMap;
    lightMap = nullptr;
}

void LightingSystem::setAmbientLevel(int level) {
    ambientLevel = Utility::clamp(level, 0, 255);
}

void LightingSystem::clearLights() {
    lights.clear();
}

void LightingSystem::addLight(const sf::Vector2f& position, float radius,
                               const sf::Color& color, float intensity,
                               float flickerAmount) {
    Light light;
    light.position = position;
    light.radius = radius;
    light.color = color;
    light.intensity = intensity;
    light.flickerAmount = flickerAmount;
    lights.push_back(light);
}

void LightingSystem::drawRadialLight(const Light& light) {
    // Draw a radial gradient using VertexArray TriangleFan.
    // Center is bright, edges fade to transparent (ambient darkness).
    const int segments = 32;
    sf::VertexArray circle(sf::TriangleFan, segments + 2);

    // Center vertex: full brightness
    circle[0].position = light.position;
    circle[0].color = sf::Color(
        static_cast<sf::Uint8>(light.color.r * light.intensity),
        static_cast<sf::Uint8>(light.color.g * light.intensity),
        static_cast<sf::Uint8>(light.color.b * light.intensity),
        255
    );

    // Edge vertices: fade to transparent (which blends to ambient)
    for (int i = 0; i <= segments; ++i) {
        float angle = (static_cast<float>(i) / segments) * 2.0f * 3.14159265f;
        float x = light.position.x + std::cos(angle) * light.radius;
        float y = light.position.y + std::sin(angle) * light.radius;
        circle[i + 1].position = sf::Vector2f(x, y);
        circle[i + 1].color = sf::Color(0, 0, 0, 0);
    }

    // Draw with additive blending onto the light map
    sf::RenderStates additive;
    additive.blendMode = sf::BlendAdd;
    lightMap->draw(circle, additive);
}

void LightingSystem::render(sf::RenderWindow& window, float dt) {
    flickerTimer += dt;

    // Fill light map with ambient darkness
    sf::Uint8 amb = static_cast<sf::Uint8>(ambientLevel);
    lightMap->clear(sf::Color(amb, amb, amb));

    // Draw each light -- Lab: STL vector iteration with STL iterators
    for (auto it = lights.begin(); it != lights.end(); ++it) {
        Light light = *it;    // Lab: STL iterator dereference

        // Apply flicker: modulate intensity with time-based multi-frequency noise
        if (light.flickerAmount > 0.0f) {
            float noise = std::sin(flickerTimer * 13.7f) * 0.3f
                        + std::sin(flickerTimer * 7.3f) * 0.2f
                        + std::sin(flickerTimer * 23.1f) * 0.1f;
            // Lab: function template (lerp<float>)
            float flicker = Utility::lerp(1.0f, 1.0f + noise, light.flickerAmount);
            light.intensity *= Utility::clamp(flicker, 0.2f, 1.5f);
        }

        drawRadialLight(light);
    }

    lightMap->display();

    // Composite light map over the scene using Multiply blend
    lightMapSprite.setTexture(lightMap->getTexture());
    window.draw(lightMapSprite, sf::BlendMultiply);
}
