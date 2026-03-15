#include "ParticleSystem.h"
#include <cstdlib>
#include <cmath>

ParticleSystem::ParticleSystem(float screenWidth, float screenHeight)
    : screenWidth(screenWidth)
    , screenHeight(screenHeight)
    , spawnTimer(0.0f)
{
}

void ParticleSystem::spawnParticle() {
    Particle p;
    p.position.x = static_cast<float>(std::rand() % static_cast<int>(screenWidth));
    p.position.y = static_cast<float>(std::rand() % static_cast<int>(screenHeight));
    p.velocity.x = (static_cast<float>(std::rand() % 100) - 50.0f) * 0.1f;  // Slow drift
    p.velocity.y = (static_cast<float>(std::rand() % 100) - 70.0f) * 0.05f;  // Slight upward bias
    p.maxLifetime = 3.0f + static_cast<float>(std::rand() % 40) * 0.1f;  // 3-7 seconds
    p.lifetime = p.maxLifetime;
    p.size = 1.0f + static_cast<float>(std::rand() % 30) * 0.1f;  // 1-4 pixels

    particles.push_back(p);     // Lab: STL list push_back
}

void ParticleSystem::update(float dt) {
    // Spawn new particles periodically
    spawnTimer += dt;
    if (spawnTimer >= 0.15f) {
        spawnTimer = 0.0f;
        spawnParticle();
    }

    // Update existing particles -- Lab: STL list iterator traversal
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->position.x += it->velocity.x * dt;   // Lab: STL iterator arrow operator
        it->position.y += it->velocity.y * dt;
        it->lifetime -= dt;

        if (it->lifetime <= 0.0f) {
            it = particles.erase(it);  // Lab: STL list erase (O(1) removal)
        }
        else {
            ++it;   // Lab: STL iterator increment
        }
    }
}

void ParticleSystem::render(sf::RenderWindow& window) {
    // Lab: STL list iteration with STL iterators
    for (auto it = particles.begin(); it != particles.end(); ++it) {
        float lifeRatio = it->lifetime / it->maxLifetime;
        int alpha = static_cast<int>(lifeRatio * 25.0f);  // Very subtle (max alpha 25)

        particleShape.setSize(sf::Vector2f(it->size, it->size));
        particleShape.setPosition(it->position);
        particleShape.setFillColor(sf::Color(180, 180, 190, alpha));

        window.draw(particleShape);
    }
}
