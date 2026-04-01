#include "AchievementSystem.h"
#include "SaveSystem.h"
#include <algorithm>

AchievementSystem::AchievementSystem(SaveSystem* saveSystem)
    : saveSystem(saveSystem)
{
    initializeAchievements();
    syncFromSave();
}

void AchievementSystem::initializeAchievements() {
    achievements.clear();

    // Run-based achievements
    achievements.push_back({"first_run", "First Steps", "Complete your first run", false, false});
    achievements.push_back({"runs_10", "Getting Warmed Up", "Complete 10 runs", false, false});
    achievements.push_back({"runs_50", "Marathon Runner", "Complete 50 runs", false, false});

    // Score achievements
    achievements.push_back({"score_100", "Triple Digits", "Score 100 points in a single run", false, false});
    achievements.push_back({"score_500", "High Achiever", "Score 500 points in a single run", false, false});
    achievements.push_back({"score_1000", "Campus Legend", "Score 1000 points in a single run", false, false});
    achievements.push_back({"score_2000", "Deadline Master", "Score 2000 points in a single run", false, false});

    // Combo achievements
    achievements.push_back({"combo_5", "Combo Starter", "Reach a 5x combo", false, false});
    achievements.push_back({"combo_10", "Combo King", "Reach a 10x combo", false, false});
    achievements.push_back({"combo_20", "Unstoppable", "Reach a 20x combo", false, false});

    // Coin achievements
    achievements.push_back({"coins_50", "Penny Pincher", "Collect 50 total coins", false, false});
    achievements.push_back({"coins_100", "Coin Collector", "Collect 100 total coins", false, false});
    achievements.push_back({"coins_500", "Rich Student", "Collect 500 total coins", false, false});

    // Skill achievements
    achievements.push_back({"dodge_100", "Nimble", "Dodge 100 total obstacles", false, false});
    achievements.push_back({"near_miss_10", "Close Call", "Get 10 near misses", false, false});

    // Special achievements
    achievements.push_back({"no_coins", "Minimalist", "Complete a run without collecting any coins", false, false});
    achievements.push_back({"perfect_run", "Perfect Run", "Score 500+ without getting hit", false, false});
}

void AchievementSystem::checkAchievements(float score, int combo, int totalCoins, int runs) {
    // Score checks
    if (score >= 100) unlock("score_100");
    if (score >= 500) unlock("score_500");
    if (score >= 1000) unlock("score_1000");
    if (score >= 2000) unlock("score_2000");

    // Combo checks
    if (combo >= 5) unlock("combo_5");
    if (combo >= 10) unlock("combo_10");
    if (combo >= 20) unlock("combo_20");

    // Coin checks
    if (totalCoins >= 50) unlock("coins_50");
    if (totalCoins >= 100) unlock("coins_100");
    if (totalCoins >= 500) unlock("coins_500");

    // Run checks
    if (runs >= 1) unlock("first_run");
    if (runs >= 10) unlock("runs_10");
    if (runs >= 50) unlock("runs_50");
}

void AchievementSystem::onRunComplete(float score, int coinsEarned, int obstaclesDodged, int maxCombo) {
    // Check for special achievements
    if (coinsEarned == 0 && score > 50) {
        unlock("no_coins");
    }
    if (score >= 500) {
        unlock("perfect_run");  // Simplified - just requires score threshold
    }
}

void AchievementSystem::onComboReached(int combo) {
    if (combo >= 5) unlock("combo_5");
    if (combo >= 10) unlock("combo_10");
    if (combo >= 20) unlock("combo_20");
}

void AchievementSystem::onScoreReached(float score) {
    if (score >= 100) unlock("score_100");
    if (score >= 500) unlock("score_500");
    if (score >= 1000) unlock("score_1000");
    if (score >= 2000) unlock("score_2000");
}

void AchievementSystem::onCoinsCollected(int totalCoins) {
    if (totalCoins >= 50) unlock("coins_50");
    if (totalCoins >= 100) unlock("coins_100");
    if (totalCoins >= 500) unlock("coins_500");
}

std::vector<Achievement> AchievementSystem::popNewlyUnlocked() {
    std::vector<Achievement> result = newlyUnlocked;
    newlyUnlocked.clear();
    return result;
}

bool AchievementSystem::isUnlocked(const std::string& id) const {
    for (const auto& ach : achievements) {
        if (ach.id == id) {
            return ach.unlocked;
        }
    }
    return false;
}

void AchievementSystem::unlock(const std::string& id) {
    for (auto& ach : achievements) {
        if (ach.id == id && !ach.unlocked) {
            ach.unlocked = true;
            ach.justUnlocked = true;
            newlyUnlocked.push_back(ach);

            // Save to persistent storage
            if (saveSystem) {
                saveSystem->unlockAchievement(id);
            }
            break;
        }
    }
}

void AchievementSystem::syncFromSave() {
    if (!saveSystem) return;

    auto unlockedIds = saveSystem->getUnlockedAchievements();
    for (const auto& id : unlockedIds) {
        for (auto& ach : achievements) {
            if (ach.id == id) {
                ach.unlocked = true;
                break;
            }
        }
    }
}

void AchievementSystem::syncToSave() {
    if (!saveSystem) return;

    for (const auto& ach : achievements) {
        if (ach.unlocked) {
            saveSystem->unlockAchievement(ach.id);
        }
    }
}
