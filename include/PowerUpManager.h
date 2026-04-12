#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <random>
#include "Constants.h"

// Types of power-ups available
enum class PowerUpType {
    Shield,       // Absorbs one hit
    SlowMotion,   // Slows down time
    DoubleScore,  // 2x score multiplier
    Magnet,       // Attracts coins
    Ammo,         // Instant ammo pickup
    UnlimitedBullets, // Temporary infinite ammo
    COUNT
};

// A single power-up entity in the game world
struct PowerUp {
    PowerUpType type;
    sf::Vector2f position;
    float speed;
    bool collected;
    float bobOffset;    // For floating animation
    float bobPhase;
};

// Tracks active power-up effects
struct ActivePowerUp {
    PowerUpType type;
    float remainingTime;
    float maxTime;
};

// Manages power-up spawning, collection, and effects.
class PowerUpManager {
public:
    PowerUpManager(float screenWidth, float groundY);

    // Spawning
    void update(float dt, float obstacleSpeed, std::mt19937& rng);
    void spawnPowerUp(PowerUpType type, float x, float y, float speed);
    void spawnRandomPowerUp(float x, float y, float speed, std::mt19937& rng);

    // Collection
    bool checkCollection(const sf::FloatRect& playerBounds, PowerUpType* collectedType = nullptr);
    void collectPowerUp(size_t index);

    // Rendering
    void render(sf::RenderWindow& window);

    // Active effects management
    void activatePowerUp(PowerUpType type);
    void updateActiveEffects(float dt);
    bool isActive(PowerUpType type) const;
    float getRemainingTime(PowerUpType type) const;

    // Effect queries
    bool hasShield() const { return isActive(PowerUpType::Shield); }
    bool isSlowMo() const { return isActive(PowerUpType::SlowMotion); }
    bool hasDoubleScore() const { return isActive(PowerUpType::DoubleScore); }
    bool hasMagnet() const { return isActive(PowerUpType::Magnet); }
    float getSlowMoFactor() const { return isSlowMo() ? Constants::POWERUP_SLOWMO_FACTOR : 1.0f; }
    float getScoreMultiplier() const { return hasDoubleScore() ? 2.0f : 1.0f; }

    // Shield consumption
    void consumeShield();

    // Clear all
    void reset();
    void clearPowerUps();
    void clearActiveEffects();

    // Get active effects for HUD
    const std::vector<ActivePowerUp>& getActiveEffects() const { return activeEffects; }

    // Accessors
    const std::vector<PowerUp>& getPowerUps() const { return powerUps; }

private:
    std::vector<PowerUp> powerUps;
    std::vector<ActivePowerUp> activeEffects;

    float screenWidth;
    float groundY;
    float spawnTimer;
    float nextSpawnDelay;

    // Get power-up color by type
    sf::Color getColor(PowerUpType type) const;
    std::string getName(PowerUpType type) const;
    float getDuration(PowerUpType type) const;

    void updatePowerUpPositions(float dt);
    void removeOffscreenPowerUps();
};
