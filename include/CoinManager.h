#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <random>
#include "Constants.h"

// A single collectible coin
struct Coin {
    sf::Vector2f position;
    float speed;
    bool collected;
    float bobPhase;
    float rotation;
    int value;
};

// Manages coin spawning, collection, and rendering.
class CoinManager {
public:
    CoinManager(float screenWidth, float groundY);

    // Update coin positions and spawn new ones
    void update(float dt, float obstacleSpeed, std::mt19937& rng);

    // Check for coin collection
    int checkCollection(const sf::FloatRect& playerBounds, bool hasMagnet, sf::Vector2f playerPos);

    // Rendering
    void render(sf::RenderWindow& window);

    // Reset all coins
    void reset();

    // Get total coins collected this run
    int getCoinsCollectedThisRun() const { return coinsCollectedThisRun; }

    // Spawn patterns
    void spawnCoinLine(float x, float y, float speed, int count, float spacing);
    void spawnCoinArc(float x, float y, float speed, int count);

private:
    std::vector<Coin> coins;
    float screenWidth;
    float groundY;
    float spawnTimer;
    float nextSpawnDelay;
    int coinsCollectedThisRun;

    void spawnCoin(float x, float y, float speed, int value = Constants::COIN_VALUE);
    void updateCoinPositions(float dt, bool hasMagnet, sf::Vector2f playerPos);
    void removeOffscreenCoins();
};
