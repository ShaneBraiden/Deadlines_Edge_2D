#include "ParticleSystem.h"
#include <cstdlib>
#include <cmath>

namespace {
    // Random float in range [min, max]
    float randomFloat(float min, float max) {
        float r = static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX);
        return min + r * (max - min);
    }
    
    // Random int in range [min, max]
    int randomInt(int min, int max) {
        return min + (std::rand() % (max - min + 1));
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
    p.velocity.x = (static_cast<float>(std::rand() % 100) - 50.0f) * 0.1f;
    p.velocity.y = (static_cast<float>(std::rand() % 100) - 70.0f) * 0.05f;
    p.maxLifetime = 3.0f + static_cast<float>(std::rand() % 40) * 0.1f;
    p.lifetime = p.maxLifetime;
    p.size = 1.0f + static_cast<float>(std::rand() % 30) * 0.1f;

    particles.push_back(p);
}

void ParticleSystem::spawnWoodDestruction(const sf::Vector2f& position) {
    // Wood colors - browns and tans
    sf::Color woodColors[] = {
        sf::Color(139, 90, 43),    // Dark wood
        sf::Color(160, 120, 70),   // Medium wood
        sf::Color(180, 140, 90),   // Light wood
        sf::Color(120, 80, 40),    // Very dark
        sf::Color(200, 160, 100)   // Pale wood
    };
    
    // Spawn wood splinters (long thin pieces)
    for (int i = 0; i < 12; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-20, 20), randomFloat(-20, 20));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(200.0f, 500.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed - randomFloat(100, 300); // Upward bias
        
        p.maxLifetime = randomFloat(0.3f, 0.6f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(3.0f, 8.0f);      // Width
        p.sizeY = randomFloat(12.0f, 25.0f);   // Length (splinters are long)
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-900.0f, 900.0f);
        p.color = woodColors[randomInt(0, 4)];
        p.debrisType = DebrisType::WoodSplinter;
        
        explosions.push_back(p);
    }
    
    // Spawn wood chunks (square pieces)
    for (int i = 0; i < 8; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-15, 15), randomFloat(-15, 15));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(150.0f, 400.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed - randomFloat(50, 200);
        
        p.maxLifetime = randomFloat(0.4f, 0.7f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(6.0f, 14.0f);
        p.sizeY = p.size * randomFloat(0.8f, 1.2f);
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-600.0f, 600.0f);
        p.color = woodColors[randomInt(0, 4)];
        p.debrisType = DebrisType::Chunk;
        
        explosions.push_back(p);
    }
    
    // Spawn dust cloud
    for (int i = 0; i < 10; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-10, 10), randomFloat(-10, 10));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(50.0f, 150.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed - randomFloat(30, 80);
        
        p.maxLifetime = randomFloat(0.3f, 0.5f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(8.0f, 20.0f);
        p.sizeY = p.size;
        p.rotation = 0;
        p.rotationSpeed = 0;
        p.color = sf::Color(180, 160, 140, 150);
        p.debrisType = DebrisType::Dust;
        
        explosions.push_back(p);
    }
    
    // Spawn bright sparks
    for (int i = 0; i < 6; ++i) {
        ExplosionParticle p;
        p.position = position;
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(300.0f, 600.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed;
        
        p.maxLifetime = randomFloat(0.1f, 0.25f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(2.0f, 5.0f);
        p.sizeY = p.size;
        p.rotation = 0;
        p.rotationSpeed = 0;
        p.color = sf::Color(255, 230, 150); // Bright yellow
        p.debrisType = DebrisType::Spark;
        
        explosions.push_back(p);
    }
}

void ParticleSystem::spawnPaperDestruction(const sf::Vector2f& position) {
    // Paper colors - whites and light grays with hints of page color
    sf::Color paperColors[] = {
        sf::Color(250, 248, 240),   // Off-white
        sf::Color(240, 235, 220),   // Cream
        sf::Color(255, 255, 255),   // White
        sf::Color(230, 225, 210),   // Aged paper
        sf::Color(245, 240, 230)    // Light cream
    };
    
    // Spawn paper fragments (flutter down)
    for (int i = 0; i < 15; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-25, 25), randomFloat(-20, 20));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(100.0f, 300.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed - randomFloat(50, 150);
        
        p.maxLifetime = randomFloat(0.5f, 1.0f);  // Paper floats longer
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(8.0f, 18.0f);
        p.sizeY = randomFloat(10.0f, 20.0f);
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-400.0f, 400.0f);  // Slower rotation
        p.color = paperColors[randomInt(0, 4)];
        p.debrisType = DebrisType::PaperFragment;
        
        explosions.push_back(p);
    }
    
    // Spawn some pages with text lines (darker fragments)
    for (int i = 0; i < 5; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-20, 20), randomFloat(-15, 15));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(80.0f, 200.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed - randomFloat(30, 100);
        
        p.maxLifetime = randomFloat(0.6f, 1.2f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(15.0f, 25.0f);
        p.sizeY = randomFloat(20.0f, 30.0f);
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-300.0f, 300.0f);
        p.color = sf::Color(245, 240, 230);  // Page base color
        p.debrisType = DebrisType::PaperFragment;
        
        explosions.push_back(p);
    }
    
    // Spawn dust puff
    for (int i = 0; i < 8; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-8, 8), randomFloat(-8, 8));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(40.0f, 120.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed - randomFloat(20, 60);
        
        p.maxLifetime = randomFloat(0.25f, 0.45f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(10.0f, 25.0f);
        p.sizeY = p.size;
        p.rotation = 0;
        p.rotationSpeed = 0;
        p.color = sf::Color(220, 215, 200, 120);
        p.debrisType = DebrisType::Dust;
        
        explosions.push_back(p);
    }
}

void ParticleSystem::spawnExplosion(const sf::Vector2f& position, const sf::Color& baseColor, int particleCount) {
    // Generic explosion - mix of chunks, sparks, and dust
    for (int i = 0; i < particleCount; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-10, 10), randomFloat(-10, 10));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(250.0f, 550.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed - randomFloat(100, 250);
        
        p.maxLifetime = randomFloat(0.2f, 0.5f);
        p.lifetime = p.maxLifetime;
        
        // Vary debris type
        int typeRoll = randomInt(0, 10);
        if (typeRoll < 4) {
            // Chunks
            p.debrisType = DebrisType::Chunk;
            p.size = randomFloat(5.0f, 12.0f);
            p.sizeY = p.size * randomFloat(0.7f, 1.3f);
            p.rotationSpeed = randomFloat(-800.0f, 800.0f);
        } else if (typeRoll < 7) {
            // Sparks
            p.debrisType = DebrisType::Spark;
            p.size = randomFloat(2.0f, 5.0f);
            p.sizeY = p.size;
            p.velocity.x *= 1.3f;
            p.velocity.y *= 1.3f;
            p.maxLifetime *= 0.6f;
            p.lifetime = p.maxLifetime;
            p.rotationSpeed = 0;
        } else {
            // Dust
            p.debrisType = DebrisType::Dust;
            p.size = randomFloat(8.0f, 18.0f);
            p.sizeY = p.size;
            p.velocity.x *= 0.5f;
            p.velocity.y *= 0.5f;
            p.rotationSpeed = 0;
        }
        
        p.rotation = randomFloat(0.0f, 360.0f);
        
        // Color variation
        int rVar = randomInt(-30, 30);
        int gVar = randomInt(-30, 30);
        int bVar = randomInt(-20, 20);
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
    for (int i = 0; i < particleCount; ++i) {
        ExplosionParticle p;
        p.position = position + sf::Vector2f(randomFloat(-5, 5), randomFloat(-5, 5));
        
        float angle = randomFloat(0.0f, 6.283f);
        float speed = randomFloat(100.0f, 250.0f);
        p.velocity.x = std::cos(angle) * speed;
        p.velocity.y = std::sin(angle) * speed;
        
        p.maxLifetime = randomFloat(0.1f, 0.25f);
        p.lifetime = p.maxLifetime;
        p.size = randomFloat(2.0f, 5.0f);
        p.sizeY = p.size;
        p.rotation = randomFloat(0.0f, 360.0f);
        p.rotationSpeed = randomFloat(-500.0f, 500.0f);
        p.debrisType = (i < particleCount / 2) ? DebrisType::Spark : DebrisType::Chunk;
        
        int var = randomInt(-20, 20);
        p.color = sf::Color(
            static_cast<sf::Uint8>(std::max(0, std::min(255, color.r + var))),
            static_cast<sf::Uint8>(std::max(0, std::min(255, color.g + var))),
            static_cast<sf::Uint8>(std::max(0, std::min(255, color.b + var))),
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

    // Update ambient particles
    for (auto it = particles.begin(); it != particles.end(); ) {
        it->position.x += it->velocity.x * dt;
        it->position.y += it->velocity.y * dt;
        it->lifetime -= dt;

        if (it->lifetime <= 0.0f) {
            it = particles.erase(it);
        } else {
            ++it;
        }
    }

    // Update explosion particles
    for (auto it = explosions.begin(); it != explosions.end(); ) {
        it->position.x += it->velocity.x * dt;
        it->position.y += it->velocity.y * dt;
        
        // Physics based on debris type
        switch (it->debrisType) {
            case DebrisType::WoodSplinter:
            case DebrisType::Chunk:
                it->velocity.y += 800.0f * dt;  // Strong gravity
                it->velocity.x *= (1.0f - 2.0f * dt);
                break;
            case DebrisType::PaperFragment:
                it->velocity.y += 200.0f * dt;  // Light gravity (paper floats)
                it->velocity.x *= (1.0f - 3.0f * dt);  // More air resistance
                // Flutter effect
                it->velocity.x += std::sin(it->lifetime * 15.0f) * 50.0f * dt;
                break;
            case DebrisType::Spark:
                it->velocity.x *= (1.0f - 5.0f * dt);  // Quick slowdown
                it->velocity.y *= (1.0f - 5.0f * dt);
                break;
            case DebrisType::Dust:
                it->velocity.y += 50.0f * dt;   // Very light gravity
                it->velocity.x *= (1.0f - 4.0f * dt);
                it->velocity.y *= (1.0f - 3.0f * dt);
                it->size += 15.0f * dt;  // Dust expands
                it->sizeY = it->size;
                break;
        }
        
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
    // Render ambient particles
    for (auto it = particles.begin(); it != particles.end(); ++it) {
        float lifeRatio = it->lifetime / it->maxLifetime;
        int alpha = static_cast<int>(lifeRatio * 25.0f);

        particleShape.setSize(sf::Vector2f(it->size, it->size));
        particleShape.setPosition(it->position);
        particleShape.setFillColor(sf::Color(180, 180, 190, alpha));

        window.draw(particleShape);
    }

    // Render explosion particles - all procedural
    for (auto it = explosions.begin(); it != explosions.end(); ++it) {
        float lifeRatio = it->lifetime / it->maxLifetime;
        sf::Uint8 alpha = static_cast<sf::Uint8>(lifeRatio * 255.0f);
        float scale = 0.3f + lifeRatio * 0.7f;
        
        switch (it->debrisType) {
            case DebrisType::WoodSplinter: {
                // Long thin rectangle
                sf::RectangleShape splinter(sf::Vector2f(it->size * scale, it->sizeY * scale));
                splinter.setOrigin(it->size * scale * 0.5f, it->sizeY * scale * 0.5f);
                splinter.setPosition(it->position);
                splinter.setRotation(it->rotation);
                splinter.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, alpha));
                window.draw(splinter);
                break;
            }
            
            case DebrisType::Chunk: {
                // Square/rectangular piece
                sf::RectangleShape chunk(sf::Vector2f(it->size * scale, it->sizeY * scale));
                chunk.setOrigin(it->size * scale * 0.5f, it->sizeY * scale * 0.5f);
                chunk.setPosition(it->position);
                chunk.setRotation(it->rotation);
                chunk.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, alpha));
                window.draw(chunk);
                break;
            }
            
            case DebrisType::PaperFragment: {
                // Paper with slight transparency and wave effect
                float waveScale = 0.9f + 0.1f * std::sin(it->lifetime * 20.0f);
                sf::RectangleShape paper(sf::Vector2f(it->size * scale * waveScale, it->sizeY * scale));
                paper.setOrigin(it->size * scale * waveScale * 0.5f, it->sizeY * scale * 0.5f);
                paper.setPosition(it->position);
                paper.setRotation(it->rotation);
                paper.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, alpha));
                window.draw(paper);
                
                // Add subtle "text lines" on larger papers
                if (it->size > 12.0f) {
                    for (int line = 0; line < 3; ++line) {
                        sf::RectangleShape textLine(sf::Vector2f(it->size * scale * 0.6f, 1.0f));
                        textLine.setOrigin(it->size * scale * 0.3f, 0.5f);
                        textLine.setPosition(it->position.x, it->position.y - it->sizeY * 0.2f + line * 4.0f);
                        textLine.setRotation(it->rotation);
                        textLine.setFillColor(sf::Color(100, 100, 100, alpha / 3));
                        window.draw(textLine);
                    }
                }
                break;
            }
            
            case DebrisType::Spark: {
                // Bright glowing circle
                float sparkSize = it->size * scale;
                
                // Outer glow
                sf::CircleShape glow(sparkSize * 1.5f);
                glow.setOrigin(sparkSize * 1.5f, sparkSize * 1.5f);
                glow.setPosition(it->position);
                glow.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, alpha / 3));
                window.draw(glow);
                
                // Core
                sf::CircleShape core(sparkSize);
                core.setOrigin(sparkSize, sparkSize);
                core.setPosition(it->position);
                core.setFillColor(sf::Color(255, 255, 230, alpha));
                window.draw(core);
                break;
            }
            
            case DebrisType::Dust: {
                // Expanding translucent circle
                sf::Uint8 dustAlpha = static_cast<sf::Uint8>(alpha * 0.4f);
                float dustSize = it->size;
                
                sf::CircleShape dust(dustSize);
                dust.setOrigin(dustSize, dustSize);
                dust.setPosition(it->position);
                dust.setFillColor(sf::Color(it->color.r, it->color.g, it->color.b, dustAlpha));
                window.draw(dust);
                break;
            }
        }
    }
}
