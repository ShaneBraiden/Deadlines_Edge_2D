#pragma once

#include <SFML/Graphics.hpp>
#include <list>

// Simple particle for ambient atmospheric effects (dust, fog).
//
// Lab requirements covered:
//   - STL list: std::list<Particle> for efficient insert/remove
//   - STL iterators: manual iteration over particle list

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;         // Seconds remaining
    float maxLifetime;      // Total lifespan (for alpha fade)
    float size;
};

class ParticleSystem {
public:
    ParticleSystem(float screenWidth, float screenHeight);

    // Spawn ambient particles each frame
    void update(float dt);

    // Render all alive particles
    void render(sf::RenderWindow& window);

private:
    std::list<Particle> particles;       // Lab: STL list
    float screenWidth;
    float screenHeight;
    float spawnTimer;
    sf::RectangleShape particleShape;    // Reusable draw shape

    void spawnParticle();
};
