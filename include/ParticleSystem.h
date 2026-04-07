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

// Debris type for different visual effects
enum class DebrisType {
    WoodSplinter,   // Long thin pieces (chairs/benches)
    PaperFragment,  // Irregular flat pieces (books)
    Spark,          // Bright small dots
    Dust,           // Small cloudy particles
    Chunk           // Square/rectangular pieces
};

// Explosion particle with color and rotation
struct ExplosionParticle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
    float size;
    float sizeY;        // For non-square shapes (splinters)
    float rotation;
    float rotationSpeed;
    sf::Color color;
    DebrisType debrisType;
};

class ParticleSystem {
public:
    ParticleSystem(float screenWidth, float screenHeight);

    // Set textures for effects (optional - system is fully procedural)
    void setTextures(sf::Texture* explosionTex, sf::Texture* hitSparkTex, sf::Texture* smokeTex);

    // Spawn ambient particles each frame
    void update(float dt);

    // Render all alive particles
    void render(sf::RenderWindow& window);

    // Spawn explosion at position with given color theme (for obstacle destruction)
    void spawnExplosion(const sf::Vector2f& position, const sf::Color& baseColor, int particleCount = 30);

    // Spawn wood destruction (chairs, benches) - splinters and chunks
    void spawnWoodDestruction(const sf::Vector2f& position);

    // Spawn paper destruction (books) - fluttering pages
    void spawnPaperDestruction(const sf::Vector2f& position);

    // Spawn smaller hit effect (when obstacle is damaged but not destroyed)
    void spawnHitEffect(const sf::Vector2f& position, const sf::Color& color, int particleCount = 10);

private:
    std::list<Particle> particles;              // Lab: STL list - ambient particles
    std::list<ExplosionParticle> explosions;    // Explosion/hit particles
    float screenWidth;
    float screenHeight;
    float spawnTimer;
    sf::RectangleShape particleShape;    // Reusable draw shape
    sf::CircleShape explosionShape;      // For explosion particles
    
    // Textures (optional, system works without them)
    sf::Texture* explosionTexture;
    sf::Texture* hitSparkTexture;
    sf::Texture* smokeTexture;

    void spawnParticle();
};
