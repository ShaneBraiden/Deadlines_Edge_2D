#pragma once

#include <string>
#include <vector>
#include <set>

// Persistent player data saved between sessions.
struct PlayerData {
    // Scores
    float bestScore = 0.0f;
    int totalRuns = 0;
    int totalObstaclesDodged = 0;
    float totalPlayTime = 0.0f;

    // Currency
    int totalCoins = 0;

    // Achievements
    std::set<std::string> unlockedAchievements;

    // Upgrades (for future shop system)
    int extraLives = 0;
    bool doubleCoinsUpgrade = false;
    bool shieldStartUpgrade = false;

    // Stats
    int highestCombo = 0;
    int totalNearMisses = 0;
};

// Handles saving and loading player progress.
class SaveSystem {
public:
    SaveSystem();

    // Load/save operations
    bool load();
    bool save();

    // Data access
    PlayerData& getData() { return data; }
    const PlayerData& getData() const { return data; }

    // Update helpers
    void updateBestScore(float score);
    void addCoins(int amount);
    bool spendCoins(int amount);   // Returns false if insufficient funds
    void incrementRuns();
    void addPlayTime(float seconds);
    void addObstaclesDodged(int count);
    void updateHighestCombo(int combo);
    void addNearMisses(int count);

    // Achievement helpers
    bool isAchievementUnlocked(const std::string& id) const;
    void unlockAchievement(const std::string& id);
    std::vector<std::string> getUnlockedAchievements() const;

    // Reset (for testing or new game)
    void reset();

private:
    PlayerData data;
    std::string savePath;

    std::string serialize() const;
    bool deserialize(const std::string& content);
};
