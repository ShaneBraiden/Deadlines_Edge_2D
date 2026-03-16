#pragma once

#include <SFML/Graphics.hpp>
#include <vector>

// A single parallax layer: a repeating texture that scrolls
// at a fraction of the camera speed for depth effect.
struct ParallaxLayer {
    sf::Sprite sprite;
    float scrollFactor;     // 0.0 = static, 1.0 = moves with camera
};

// Multi-layer parallax background for depth effect.
// Layers are drawn back-to-front (index 0 = farthest).
// Each layer scrolls at scrollFactor * camera position.
//
// Lab requirements covered:
//   - STL vector: std::vector<ParallaxLayer>
//   - STL iterators: explicit iteration over layers
class ParallaxBackground {
public:
    ParallaxBackground() = default;

    // Add a layer. scrollFactor: 0.0 (static bg) to 0.9 (near foreground).
    // Texture must have setRepeated(true) before calling this.
    void addLayer(sf::Texture& texture, float scrollFactor);

    // Update and render all layers based on current camera view.
    // Must be called AFTER setting the gameplay view, BEFORE drawing tilemap/entities.
    void render(sf::RenderWindow& window, const sf::View& cameraView);

private:
    std::vector<ParallaxLayer> layers;   // Lab: STL vector
};
