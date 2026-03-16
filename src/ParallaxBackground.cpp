#include "ParallaxBackground.h"

void ParallaxBackground::addLayer(sf::Texture& texture, float scrollFactor) {
    ParallaxLayer layer;
    layer.sprite.setTexture(texture);
    layer.scrollFactor = scrollFactor;
    layers.push_back(layer);
}

void ParallaxBackground::render(sf::RenderWindow& window, const sf::View& cameraView) {
    float cameraX = cameraView.getCenter().x;
    float cameraY = cameraView.getCenter().y;
    float viewW = cameraView.getSize().x;
    float viewH = cameraView.getSize().y;

    // Lab: STL vector iteration with STL iterators
    for (auto it = layers.begin(); it != layers.end(); ++it) {
        float parallaxOffset = cameraX * it->scrollFactor;

        const sf::Texture* tex = it->sprite.getTexture();
        if (!tex) continue;

        int texW = static_cast<int>(tex->getSize().x);
        int texH = static_cast<int>(tex->getSize().y);
        if (texW == 0 || texH == 0) continue;

        // Scroll by shifting the texture rect horizontally
        int offsetPx = static_cast<int>(parallaxOffset) % texW;
        if (offsetPx < 0) offsetPx += texW;

        it->sprite.setTextureRect(sf::IntRect(
            offsetPx, 0,
            static_cast<int>(viewW), static_cast<int>(viewH)
        ));

        // Position the sprite to cover the visible camera area
        it->sprite.setPosition(cameraX - viewW / 2.0f, cameraY - viewH / 2.0f);

        window.draw(it->sprite);
    }
}
