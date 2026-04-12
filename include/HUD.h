#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <deque>
#include "Constants.h"

class AssetManager;

// Toast notification for achievements, milestones, etc.
struct Toast {
    std::string message;
    sf::Color color;
    float lifetime;
    float maxLifetime;
    float slideOffset;  // For slide-in animation
};

// Active power-up display info
struct PowerUpDisplay {
    std::string name;
    sf::Color color;
    float remainingTime;
    float maxTime;
};

// Modern HUD system with combo tracking, power-ups, and dynamic feedback.
class HUD {
public:
    HUD(sf::RenderWindow& window, AssetManager* assets);

    // Core updates
    void update(float dt);

    // Rendering
    void render(sf::RenderWindow& window, sf::Font& font);

    // Score display
    void setScore(float score);
    void setBestScore(float best);
    void setCoins(int coins);
    void setAmmo(int ammoCount, int maxAmmo, bool unlimitedAmmo);

    // Combo system
    void setCombo(int combo, float multiplier);
    void triggerComboFlash();

    // Distance/progress
    void setDistance(float distance);
    void setMilestone(const std::string& milestone);

    // Power-up indicators
    void addPowerUp(const std::string& name, sf::Color color, float duration);
    void updatePowerUp(const std::string& name, float remainingTime);
    void removePowerUp(const std::string& name);
    void clearPowerUps();

    // Speed indicator
    void setSpeed(float speed, float maxSpeed);

    // Notifications
    void showToast(const std::string& message, sf::Color color = Constants::UI_TEXT_LIGHT);
    void showAchievementToast(const std::string& achievement);
    void showMilestoneToast(const std::string& milestone);

    // Screen effects
    void triggerDamageFlash();
    void triggerScorePopup(const std::string& text, sf::Vector2f position);

private:
    sf::RenderWindow& window;
    AssetManager* assets;

    // Current values
    float displayScore;       // Smoothly interpolated score
    float targetScore;
    float bestScore;
    int coins;
    int combo;
    int ammoCount;
    int maxAmmo;
    bool unlimitedAmmo;
    float comboMultiplier;
    float distance;
    float speed;
    float maxSpeed;
    std::string currentMilestone;
    void renderAmmo();

    // Animation state
    float comboFlashTimer;
    float damageFlashTimer;
    float pulseTimer;

    // Power-ups
    std::vector<PowerUpDisplay> activePowerUps;

    // Toasts
    std::deque<Toast> toasts;
    static constexpr int MAX_TOASTS = 3;

    // Score popups
    struct ScorePopup {
        std::string text;
        sf::Vector2f position;
        float lifetime;
        float alpha;
    };
    std::vector<ScorePopup> scorePopups;

    // Rendering helpers
    void renderScore();
    void renderCombo();
    void renderPowerUps();
    void renderSpeed();
    void renderDistance();
    void renderCoins();
    void renderToasts();
    void renderDamageOverlay();
    void renderScorePopups();

    // Drawing utilities
    sf::RectangleShape createProgressBar(float x, float y, float width, float height,
                                         float progress, sf::Color fillColor, sf::Color bgColor);
};
