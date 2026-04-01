#pragma once

#include "Constants.h"

// Tracks combo chains and score multipliers.
// Combos increase with successful obstacle dodges and decay over time.
class ComboSystem {
public:
    ComboSystem();

    // Call when player successfully dodges an obstacle
    void registerDodge();

    // Call when player gets a near-miss (close dodge)
    void registerNearMiss();

    // Call when combo should break (damage, etc.)
    void breakCombo();

    // Update timers
    void update(float dt);

    // Reset all state
    void reset();

    // Accessors
    int getComboCount() const { return comboCount; }
    float getMultiplier() const { return multiplier; }
    float getComboTimer() const { return comboTimer; }
    bool isComboActive() const { return comboCount > 1; }
    int getNearMissCount() const { return nearMissCount; }

    // Calculate score bonus for an action
    int calculateBonus(int basePoints) const;

    // Check if a new milestone was just reached
    bool hasNewMilestone() const { return newMilestone; }
    void clearMilestoneFlag() { newMilestone = false; }
    int getMilestoneValue() const { return milestoneValue; }

private:
    int comboCount;
    float multiplier;
    float comboTimer;
    int nearMissCount;
    int totalDodges;

    // Milestone tracking
    bool newMilestone;
    int milestoneValue;

    void updateMultiplier();
    void checkMilestone();
};
