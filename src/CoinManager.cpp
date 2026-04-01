#include "CoinManager.h"
#include <algorithm>
#include <cmath>

CoinManager::CoinManager(float screenWidth, float groundY)
    : screenWidth(screenWidth)
    , groundY(groundY)
    , spawnTimer(0.0f)
    , nextSpawnDelay(Constants::COIN_SPAWN_INTERVAL_MAX)
    , coinsCollectedThisRun(0)
{
}

void CoinManager::update(float dt, float obstacleSpeed, std::mt19937& rng) {
    // Update spawn timer
    spawnTimer += dt;
    if (spawnTimer >= nextSpawnDelay) {
        // Decide on spawn pattern
        std::uniform_int_distribution<int> patternDist(0, 100);
        int pattern = patternDist(rng);

        float spawnX = screenWidth + 50.0f;
        float spawnY = groundY - 60.0f;

        if (pattern < 40) {
            // Single coin
            spawnCoin(spawnX, spawnY, obstacleSpeed);
        } else if (pattern < 70) {
            // Line of coins
            std::uniform_int_distribution<int> countDist(3, 6);
            spawnCoinLine(spawnX, spawnY, obstacleSpeed, countDist(rng), 50.0f);
        } else {
            // Arc pattern
            spawnCoinArc(spawnX, spawnY, obstacleSpeed, 5);
        }

        spawnTimer = 0.0f;
        std::uniform_real_distribution<float> delayDist(
            Constants::COIN_SPAWN_INTERVAL_MIN,
            Constants::COIN_SPAWN_INTERVAL_MAX
        );
        nextSpawnDelay = delayDist(rng);
    }

    updateCoinPositions(dt, false, sf::Vector2f(0, 0));
    removeOffscreenCoins();
}

void CoinManager::spawnCoin(float x, float y, float speed, int value) {
    Coin coin;
    coin.position = sf::Vector2f(x, y);
    coin.speed = speed;
    coin.collected = false;
    coin.bobPhase = static_cast<float>(coins.size()) * 0.5f;  // Offset phase for variety
    coin.rotation = 0.0f;
    coin.value = value;
    coins.push_back(coin);
}

void CoinManager::spawnCoinLine(float x, float y, float speed, int count, float spacing) {
    for (int i = 0; i < count; ++i) {
        spawnCoin(x + i * spacing, y, speed);
    }
}

void CoinManager::spawnCoinArc(float x, float y, float speed, int count) {
    float arcHeight = 80.0f;
    float arcWidth = (count - 1) * 50.0f;

    for (int i = 0; i < count; ++i) {
        float t = count > 1 ? static_cast<float>(i) / (count - 1) : 0.5f;
        float arcY = y - std::sin(t * 3.14159f) * arcHeight;
        spawnCoin(x + i * 50.0f, arcY, speed);
    }
}

int CoinManager::checkCollection(const sf::FloatRect& playerBounds, bool hasMagnet, sf::Vector2f playerPos) {
    int collected = 0;

    for (auto& coin : coins) {
        if (coin.collected) continue;

        sf::FloatRect coinBounds(
            coin.position.x - 15.0f,
            coin.position.y - 15.0f,
            30.0f,
            30.0f
        );

        // Expanded collection radius with magnet
        if (hasMagnet) {
            float dx = playerPos.x - coin.position.x;
            float dy = playerPos.y - coin.position.y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance < Constants::POWERUP_MAGNET_RANGE) {
                // Move coin toward player
                float attractSpeed = 800.0f;
                float factor = 1.0f - (distance / Constants::POWERUP_MAGNET_RANGE);
                coin.position.x += dx / distance * attractSpeed * factor * 0.016f;
                coin.position.y += dy / distance * attractSpeed * factor * 0.016f;
            }
        }

        if (playerBounds.intersects(coinBounds)) {
            coin.collected = true;
            collected += coin.value;
            coinsCollectedThisRun += coin.value;
        }
    }

    return collected;
}

void CoinManager::render(sf::RenderWindow& window) {
    for (const auto& coin : coins) {
        if (coin.collected) continue;

        float y = coin.position.y + std::sin(coin.bobPhase) * Constants::COIN_FLOAT_AMPLITUDE;

        // Glow
        sf::CircleShape glow(18.0f);
        glow.setFillColor(sf::Color(255, 215, 0, 60));
        glow.setOrigin(18.0f, 18.0f);
        glow.setPosition(coin.position.x, y);
        window.draw(glow);

        // Coin shape (oval to simulate rotation)
        float scaleX = std::abs(std::cos(coin.rotation));
        scaleX = std::max(0.2f, scaleX);  // Don't let it disappear completely

        sf::CircleShape coinShape(12.0f);
        coinShape.setFillColor(sf::Color(255, 215, 0));
        coinShape.setOutlineColor(sf::Color(200, 160, 0));
        coinShape.setOutlineThickness(2.0f);
        coinShape.setOrigin(12.0f, 12.0f);
        coinShape.setScale(scaleX, 1.0f);
        coinShape.setPosition(coin.position.x, y);
        window.draw(coinShape);

        // Inner shine
        sf::CircleShape shine(6.0f);
        shine.setFillColor(sf::Color(255, 240, 150, 180));
        shine.setOrigin(6.0f, 6.0f);
        shine.setScale(scaleX, 1.0f);
        shine.setPosition(coin.position.x - 2.0f * scaleX, y - 2.0f);
        window.draw(shine);
    }
}

void CoinManager::reset() {
    coins.clear();
    spawnTimer = 0.0f;
    nextSpawnDelay = Constants::COIN_SPAWN_INTERVAL_MAX;
    coinsCollectedThisRun = 0;
}

void CoinManager::updateCoinPositions(float dt, bool hasMagnet, sf::Vector2f playerPos) {
    for (auto& coin : coins) {
        coin.position.x -= coin.speed * dt;
        coin.bobPhase += dt * Constants::COIN_FLOAT_SPEED;
        coin.rotation += dt * 5.0f;  // Spin animation
    }
}

void CoinManager::removeOffscreenCoins() {
    coins.erase(
        std::remove_if(coins.begin(), coins.end(),
            [](const Coin& coin) {
                return coin.collected || coin.position.x < -50.0f;
            }),
        coins.end()
    );
}
