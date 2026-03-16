#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include "Utility.h"

// A single light source for one frame.
struct Light {
    sf::Vector2f position;     // Screen-space position (pixels)
    float radius;              // Light radius in pixels
    sf::Color color;           // Light color
    float intensity;           // 0.0 to 1.0
    float flickerAmount;       // 0.0 = steady, 1.0 = heavy flicker
};

// Renders a light map using sf::RenderTexture + multiply blend.
// All lights are drawn as radial gradients onto a dark texture,
// then composited over the scene to create darkness with lit areas.
//
// Lab requirements covered:
//   - STL vector: std::vector<Light>
//   - STL iterators: iteration over lights
//   - Single-level pointer: sf::RenderTexture* (raw new for lab)
//   - Function templates: Utility::lerp<float> for flicker
class LightingSystem {
public:
    LightingSystem(unsigned int width, unsigned int height);
    ~LightingSystem();

    // Set the ambient darkness level (0 = pitch black, 255 = fully lit)
    void setAmbientLevel(int level);

    // Clear all lights (call each frame before adding)
    void clearLights();

    // Add a light for this frame
    void addLight(const sf::Vector2f& position, float radius,
                  const sf::Color& color, float intensity,
                  float flickerAmount = 0.0f);

    // Render the light map and composite over the scene.
    // Must be called after all scene drawing is done (in default/screen view).
    void render(sf::RenderWindow& window, float dt);

private:
    sf::RenderTexture* lightMap;      // Lab: single-level pointer via new
    sf::Sprite lightMapSprite;
    std::vector<Light> lights;        // Lab: STL vector
    int ambientLevel;                 // Base darkness (default: 25 for horror)
    float flickerTimer;               // Accumulated time for noise-based flicker

    void drawRadialLight(const Light& light);
};
