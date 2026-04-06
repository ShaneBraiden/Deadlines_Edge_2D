#include "ParticleSystem.h"
#include <cstdlib>
#include <cmath>

namespace {
    // Random float in range [min, max]
    float randomFloat(float min, float max) {
        float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        return min + r * (max - min);
    }
}

ParticleSystem::ParticleSystem(float screenWidth, float screenHeight)
    : screenWidth(screenWidth)
    , screenHeight(screenHeight)
    , spawnTimer(0.0f)
    , explosionTexture(nullptr)
    , hitSparkTexture(nullptr)
    , smokeTexture(nullptr)
{
}

void ParticleSystem::setTextures(sf::Texture* explosionTex, sf::Texture* hitSparkTex, sf::Texture* smokeTex) {
    explosionTexture = explosionTex;
    hitSparkTexture = hitSparkTex;
    smokeTexture = smokeTex;
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

void ParticleSystem::spawnExplosion(const sf::Vector2f& position, const sf::Color& baseColor, int particleCount) {
    // Spawn a few texture-based explosion sprites for visual impact
    int texturedCount = (explosionTexture != nullptr) ? std::min(3, particleCount / 5) : 0;
    
    for (int i = 0; i < particleCount; ++i) {
        ExplosionParticle p;
        p.position = position;
        
        // Random direction with varying speed
        float angle = randomFloat(0.0f, 6.283f);  // 0 to 2*PI
        float speed = randomFloat(150.0f, 450.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed;
        
        // Lifetime varies
        p.maxLifetime = randomFloat(0.3f, 0.8f);
        p.lifetime = p.maxLifetime;
        
        // Size varies based on particle type
        p.isDebris = (i < particleCount / 2);  // Half are debris
        p.useTexture = (i < texturedCount);  // First few use texture
        
        if (p.useTexture) {
            p.size = randomFloat(40.0f, 80.0f);  // Larger for texture sprites
            p.velocity.x *= 0.3f;  // Slower movement
            p.velocity.y *= 0.3f;
            p.maxLifetime = randomFloat(0.2f, 0.4f);
            p.lifetime = p.maxLifetime;
        } else if (p.isDebris) {
            p.size = randomFloat(4.0f, 12.0f);
            p.velocity.x *= 0.7f;  // Debris moves slower
            p.velocity.y *= 0.7f;
            p.maxLifetime *= 1.3f;  // Debris lasts longer
            p.lifetime = p.maxLifetime;
        } else {
            p.size = randomFloat(2.0f, 6.0f);  // Sparks are smaller
        }
        
        // Rotation
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-720.0f, 720.0f);  // Degrees per second
        
        // Color variation based on base color
        int rVar = static_cast<int>(randomFloat(-30.0f, 30.0f));
        int gVar = static_cast<int>(randomFloat(-30.0f, 30.0f));
        int bVar = static_cast<int>(randomFloat(-20.0f, 20.0f));
        p.color = sf::Color(
            static_cast<sf::Uint8>(std::max(0, std::min(255, baseColor.r + rVar))),
            static_cast<sf::Uint8>(std::max(0, std::min(255, baseColor.g + gVar))),
            static_cast<sf::Uint8>(std::max(0, std::min(255, baseColor.b + bVar))),
            255
        );
        
        explosions.push_back(p);
    }
}

void ParticleSystem::spawnHitEffect(const sf::Vector2f& position, const sf::Color& color, int particleCount) {
    // Spawn one texture-based hit spark if available
    bool useHitTexture = (hitSparkTexture != nullptr);
    
    for (int i = 0; i < particleCount; ++i) {
        ExplosionParticle p;
        p.position = position;
        
        // Random direction - smaller spread for hit effect
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(80.0f, 200.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed;
        
        p.maxLifetime = randomFloat(0.15f, 0.35f);
        p.lifetime = p.maxLifetime;
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-500.0f, 500.0f);
        p.isDebris = false;
        
        // First particle uses texture if available
        p.useTexture = (i == 0 && useHitTexture);
        if (p.useTexture) {
            p.size = randomFloat(30.0f, 50.0f);
            p.velocity.x *= 0.2f;
            p.velocity.y *= 0.2f;
        } else {
            p.size = randomFloat(2.0f, 5.0f);
        }
        
        // Slight color variation
        int variation = static_cast<int>(randomFloat(-20.0f, 20.0f));
        p.color = sf::Color(
            static_cast<sf::Uint8>(std::max(0, std::min(255, color.r + variation))),
            static_cast<sf::Uint8>(std::max(0, std::min(255, color.g + variation))),
            static_cast<sf::Uint8>(std::max(0, std::min(255, color.b + variation))),
            255
        );
        
        explosions.push_back(p);
    }
}

void ParticleSystem::update(float dt) {
    // Spawn new ambient particles periodically
    spawnTimer += dt;
    if (spawnTimer >= 0.15f) {
        spawnTimer = 0.0f;
        spawnParticle();
    }

    // Update ambient particles -- Lab: STL list iterator traversal
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

    // Update explosion particles
    const float gravity = 600.0f;  // Gravity for debris
    for (auto it = explosions.begin(); it != explosions.end(); ) {
        it->position.x += it->velocity.x * dt;
        it->position.y += it->velocity.y * dt;
        
        // Apply gravity to debris particles
        if (it->isDebris) {
            it->velocity.y += gravity * dt;
        }
        
        // Apply drag to slow particles
        it->velocity.x *= (1.0f - 2.0f * dt);
        it->velocity.y *= (1.0f - 1.5f * dt);
        
        // Update rotation
        it->rotation += it->rotationSpeed * dt;
        
        it->lifetime -= dt;

        if (it->lifetime <= 0.0f) {
            it = explosions.erase(it);
        } else {
            ++it;
        }
    }
}

void ParticleSystem::render(sf::RenderWindow& window) {
    // Render ambient particles - Lab: STL list iteration with STL iterators
    for (auto it = particles.begin(); it != particles.end(); ++it) {
        float lifeRatio = it->lifetime / it->maxLifetime;
        int alpha = static_cast<int>(lifeRatio * 25.0f);  // Very subtle (max alpha 25)

        particleShape.setSize(sf::Vector2f(it->size, it->size));
        particleShape.setPosition(it->position);
        particleShape.setFillColor(sf::Color(180, 180, 190, alpha));

        window.draw(particleShape);
    }

    // Render explosion particles
    for (auto it = explosions.begin(); it != explosions.end(); ++it) {
        float lifeRatio = it->lifetime / it->maxLifetime;
        sf::Uint8 alpha = static_cast<sf::Uint8>(lifeRatio * 255.0f);
        
        // Scale down as particle dies
        float scale = 0.3f + lifeRatio * 0.7f;
        float drawSize = it->size * scale;

        if (it->isDebris) {
            // Draw debris as rotating squares
            sf::RectangleShape debris(sf::Vector2f(drawSize, drawSize));
            debris.setOrigin(drawSize * 0.5f, drawSize * 0.5f);
            debris.setPosition(it->position);
            debris.setRotation(it->rotation);
            debris.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, alpha));
            window.draw(debris);
        } else {
            // Draw sparks as circles with glow
            explosionShape.setRadius(drawSize);
            explosionShape.setOrigin(drawSize, drawSize);
            explosionShape.setPosition(it->position);
            explosionShape.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, alpha));
            window.draw(explosionShape);
            
            // Inner bright core
            if (drawSize > 2.0f) {
                float coreSize = drawSize * 0.5f;
                sf::CircleShape core(coreSize);
                core.setOrigin(coreSize, coreSize);
                core.setPosition(it->position);
                core.setFillColor(sf::Color(255, 255, 220, alpha));
                window.draw(core);
            }
        }
    }
}
