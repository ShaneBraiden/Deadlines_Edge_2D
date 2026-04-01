#pragma once

#include <string>
#include <vector>
#include <functional>
#include "Constants.h"

// Forward declaration
class SaveSystem;
class HUD;

// Represents a single achievement
struct Achievement {
    std::string id;
    std::string name;
    std::string description;
    bool unlocked;
    bool justUnlocked;  // For notification
};

// Manages achievement tracking and unlocking.
class AchievementSystem {
public:
    AchievementSystem(SaveSystem* saveSystem);

    // Check achievements based on current game state
    void checkAchievements(float score, int combo, int totalCoins, int runs);

    // Manually trigger achievement check
    void onRunComplete(float score, int coinsEarned, int obstaclesDodged, int maxCombo);
    void onComboReached(int combo);
    void onScoreReached(float score);
    void onCoinsCollected(int totalCoins);

    // Get newly unlocked achievements (clears the queue)
    std::vector<Achievement> popNewlyUnlocked();

    // Get all achievements for display
    const std::vector<Achievement>& getAllAchievements() const { return achievements; }

    // Check specific achievement
    bool isUnlocked(const std::string& id) const;

    // Sync with save system
    void syncFromSave();
    void syncToSave();

private:
    SaveSystem* saveSystem;
    std::vector<Achievement> achievements;
    std::vector<Achievement> newlyUnlocked;

    void initializeAchievements();
    void unlock(const std::string& id);
};
