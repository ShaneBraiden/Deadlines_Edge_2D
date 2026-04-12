#include "PowerUpManager.h"
#include <algorithm>
#include <cmath>

PowerUpManager::PowerUpManager(float screenWidth, float groundY)
    : screenWidth(screenWidth)
    , groundY(groundY)
    , spawnTimer(0.0f)
    , nextSpawnDelay(Constants::POWERUP_SPAWN_INTERVAL_MAX)
{
}

void PowerUpManager::update(float dt, float obstacleSpeed, std::mt19937& rng) {
    // Update spawn timer
    spawnTimer += dt;
    if (spawnTimer >= nextSpawnDelay) {
        // Spawn a random power-up
        float spawnX = screenWidth + 100.0f;
        float spawnY = groundY - 80.0f;  // Above ground
        spawnRandomPowerUp(spawnX, spawnY, obstacleSpeed, rng);

        spawnTimer = 0.0f;
        std::uniform_real_distribution<float> delayDist(
            Constants::POWERUP_SPAWN_INTERVAL_MIN,
            Constants::POWERUP_SPAWN_INTERVAL_MAX
        );
        nextSpawnDelay = delayDist(rng);
    }

    updatePowerUpPositions(dt);
    removeOffscreenPowerUps();
    updateActiveEffects(dt);
}

void PowerUpManager::spawnPowerUp(PowerUpType type, float x, float y, float speed) {
    PowerUp pu;
    pu.type = type;
    pu.position = sf::Vector2f(x, y);
    pu.speed = speed;
    pu.collected = false;
    pu.bobOffset = 0.0f;
    pu.bobPhase = 0.0f;
    powerUps.push_back(pu);
}

void PowerUpManager::spawnRandomPowerUp(float x, float y, float speed, std::mt19937& rng) {
    std::uniform_int_distribution<int> typeDist(0, static_cast<int>(PowerUpType::COUNT) - 1);
    PowerUpType type = static_cast<PowerUpType>(typeDist(rng));
    spawnPowerUp(type, x, y, speed);
}

bool PowerUpManager::checkCollection(const sf::FloatRect& playerBounds, PowerUpType* collectedType) {
    for (size_t i = 0; i < powerUps.size(); ++i) {
        if (powerUps[i].collected) continue;

        sf::FloatRect powerUpBounds(
            powerUps[i].position.x - 20.0f,
            powerUps[i].position.y - 20.0f + powerUps[i].bobOffset,
            40.0f,
            40.0f
        );

        if (playerBounds.intersects(powerUpBounds)) {
            if (collectedType) {
                *collectedType = powerUps[i].type;
            }
            collectPowerUp(i);
            return true;
        }
    }
    return false;
}

void PowerUpManager::collectPowerUp(size_t index) {
    if (index >= powerUps.size()) return;

    PowerUp& pu = powerUps[index];
    pu.collected = true;

    switch (pu.type) {
        case PowerUpType::Shield:
        case PowerUpType::SlowMotion:
        case PowerUpType::DoubleScore:
        case PowerUpType::Magnet:
        case PowerUpType::UnlimitedBullets:
            activatePowerUp(pu.type);
            break;
        case PowerUpType::Ammo:
            break;
    }
}

void PowerUpManager::render(sf::RenderWindow& window) {
    for (const auto& pu : powerUps) {
        if (pu.collected) continue;

        float y = pu.position.y + pu.bobOffset;

        // Glow effect
        sf::CircleShape glow(30.0f);
        sf::Color glowColor = getColor(pu.type);
        glowColor.a = 50;
        glow.setFillColor(glowColor);
        glow.setOrigin(30.0f, 30.0f);
        glow.setPosition(pu.position.x, y);
        window.draw(glow);

        // Main shape
        sf::CircleShape shape(20.0f);
        shape.setFillColor(getColor(pu.type));
        shape.setOutlineColor(sf::Color::White);
        shape.setOutlineThickness(2.0f);
        shape.setOrigin(20.0f, 20.0f);
        shape.setPosition(pu.position.x, y);
        window.draw(shape);

        // Simple icon (just a letter shape based on type)
        // Draw inner symbol based on type
        sf::Color symbolColor = sf::Color::White;
        switch (pu.type) {
            case PowerUpType::Shield: {
                // Shield icon - hexagon-ish
                sf::CircleShape inner(12.0f, 6);
                inner.setFillColor(sf::Color(255, 255, 255, 200));
                inner.setOrigin(12.0f, 12.0f);
                inner.setPosition(pu.position.x, y);
                window.draw(inner);
                break;
            }
            case PowerUpType::SlowMotion: {
                // Clock icon - circle with line
                sf::CircleShape inner(10.0f);
                inner.setFillColor(sf::Color::Transparent);
                inner.setOutlineColor(sf::Color::White);
                inner.setOutlineThickness(2.0f);
                inner.setOrigin(10.0f, 10.0f);
                inner.setPosition(pu.position.x, y);
                window.draw(inner);
                // Clock hand
                sf::RectangleShape hand(sf::Vector2f(2.0f, 8.0f));
                hand.setFillColor(sf::Color::White);
                hand.setOrigin(1.0f, 8.0f);
                hand.setPosition(pu.position.x, y);
                hand.setRotation(45.0f);
                window.draw(hand);
                break;
            }
            case PowerUpType::DoubleScore: {
                // Star shape - just use a smaller circle
                sf::CircleShape inner(8.0f, 5);
                inner.setFillColor(sf::Color::White);
                inner.setOrigin(8.0f, 8.0f);
                inner.setPosition(pu.position.x, y);
                window.draw(inner);
                break;
            }
            case PowerUpType::Magnet: {
                // U-shape magnet simplified as two rectangles
                sf::RectangleShape left(sf::Vector2f(4.0f, 14.0f));
                left.setFillColor(sf::Color::White);
                left.setPosition(pu.position.x - 7.0f, y - 7.0f);
                window.draw(left);
                sf::RectangleShape right(sf::Vector2f(4.0f, 14.0f));
                right.setFillColor(sf::Color::White);
                right.setPosition(pu.position.x + 3.0f, y - 7.0f);
                window.draw(right);
                sf::RectangleShape bottom(sf::Vector2f(14.0f, 4.0f));
                bottom.setFillColor(sf::Color::White);
                bottom.setPosition(pu.position.x - 7.0f, y + 3.0f);
                window.draw(bottom);
                break;
            }
            case PowerUpType::Ammo: {
                sf::RectangleShape body(sf::Vector2f(18.0f, 12.0f));
                body.setFillColor(sf::Color::White);
                body.setOrigin(9.0f, 6.0f);
                body.setPosition(pu.position.x, y);
                window.draw(body);
                break;
            }
            case PowerUpType::UnlimitedBullets: {
                sf::CircleShape inner(8.0f, 3);
                inner.setFillColor(sf::Color::White);
                inner.setOrigin(8.0f, 8.0f);
                inner.setPosition(pu.position.x, y);
                window.draw(inner);
                break;
            }
            default:
                break;
        }
    }
}

void PowerUpManager::activatePowerUp(PowerUpType type) {
    // Check if already active - refresh duration
    for (auto& effect : activeEffects) {
        if (effect.type == type) {
            effect.remainingTime = getDuration(type);
            effect.maxTime = getDuration(type);
            return;
        }
    }

    // Add new effect
    ActivePowerUp effect;
    effect.type = type;
    effect.remainingTime = getDuration(type);
    effect.maxTime = getDuration(type);
    activeEffects.push_back(effect);
}

void PowerUpManager::updateActiveEffects(float dt) {
    for (auto it = activeEffects.begin(); it != activeEffects.end();) {
        it->remainingTime -= dt;
        if (it->remainingTime <= 0.0f) {
            it = activeEffects.erase(it);
        } else {
            ++it;
        }
    }
}

bool PowerUpManager::isActive(PowerUpType type) const {
    for (const auto& effect : activeEffects) {
        if (effect.type == type && effect.remainingTime > 0.0f) {
            return true;
        }
    }
    return false;
}

float PowerUpManager::getRemainingTime(PowerUpType type) const {
    for (const auto& effect : activeEffects) {
        if (effect.type == type) {
            return effect.remainingTime;
        }
    }
    return 0.0f;
}

void PowerUpManager::consumeShield() {
    for (auto it = activeEffects.begin(); it != activeEffects.end(); ++it) {
        if (it->type == PowerUpType::Shield) {
            activeEffects.erase(it);
            return;
        }
    }
}

void PowerUpManager::reset() {
    clearPowerUps();
    clearActiveEffects();
    spawnTimer = 0.0f;
    nextSpawnDelay = Constants::POWERUP_SPAWN_INTERVAL_MAX;
}

void PowerUpManager::clearPowerUps() {
    powerUps.clear();
}

void PowerUpManager::clearActiveEffects() {
    activeEffects.clear();
}

sf::Color PowerUpManager::getColor(PowerUpType type) const {
    switch (type) {
        case PowerUpType::Shield:      return sf::Color(100, 180, 255);   // Light blue
        case PowerUpType::SlowMotion:  return sf::Color(180, 100, 255);   // Purple
        case PowerUpType::DoubleScore: return sf::Color(255, 215, 0);     // Gold
        case PowerUpType::Magnet:      return sf::Color(255, 100, 150);   // Pink
        case PowerUpType::Ammo:        return sf::Color(245, 245, 245);   // White
        case PowerUpType::UnlimitedBullets: return sf::Color(80, 220, 255); // Cyan
        default:                       return sf::Color::White;
    }
}

std::string PowerUpManager::getName(PowerUpType type) const {
    switch (type) {
        case PowerUpType::Shield:      return "Shield";
        case PowerUpType::SlowMotion:  return "SlowMo";
        case PowerUpType::DoubleScore: return "Double";
        case PowerUpType::Magnet:      return "Magnet";
        case PowerUpType::Ammo:        return "Ammo";
        case PowerUpType::UnlimitedBullets: return "Unlimited";
        default:                       return "Unknown";
    }
}

float PowerUpManager::getDuration(PowerUpType type) const {
    switch (type) {
        case PowerUpType::Shield:      return Constants::POWERUP_SHIELD_DURATION;
        case PowerUpType::SlowMotion:  return Constants::POWERUP_SLOWMO_DURATION;
        case PowerUpType::DoubleScore: return Constants::POWERUP_DOUBLESCORE_DURATION;
        case PowerUpType::Magnet:      return Constants::POWERUP_MAGNET_DURATION;
        case PowerUpType::Ammo:        return 0.0f;
        case PowerUpType::UnlimitedBullets: return Constants::POWERUP_UNLIMITED_BULLETS_DURATION;
        default:                       return 5.0f;
    }
}

void PowerUpManager::updatePowerUpPositions(float dt) {
    for (auto& pu : powerUps) {
        pu.position.x -= pu.speed * dt;

        // Bobbing animation
        pu.bobPhase += dt * Constants::COIN_FLOAT_SPEED;
        pu.bobOffset = std::sin(pu.bobPhase) * Constants::COIN_FLOAT_AMPLITUDE;
    }
}

void PowerUpManager::removeOffscreenPowerUps() {
    powerUps.erase(
        std::remove_if(powerUps.begin(), powerUps.end(),
            [](const PowerUp& pu) {
                return pu.collected || pu.position.x < -50.0f;
            }),
        powerUps.end()
    );
}
