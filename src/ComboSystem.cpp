#include "ComboSystem.h"
#include <algorithm>

ComboSystem::ComboSystem()
    : comboCount(0)
    , multiplier(1.0f)
    , comboTimer(0.0f)
    , nearMissCount(0)
    , totalDodges(0)
    , newMilestone(false)
    , milestoneValue(0)
{
}

void ComboSystem::registerDodge() {
    comboCount++;
    totalDodges++;
    comboTimer = Constants::COMBO_TIMEOUT;
    updateMultiplier();
    checkMilestone();
}

void ComboSystem::registerNearMiss() {
    nearMissCount++;
    comboCount += 2;  // Near-miss worth more for combo
    comboTimer = Constants::COMBO_TIMEOUT;
    updateMultiplier();
    checkMilestone();
}

void ComboSystem::breakCombo() {
    comboCount = 0;
    multiplier = 1.0f;
    comboTimer = 0.0f;
}

void ComboSystem::update(float dt) {
    if (comboTimer > 0.0f) {
        comboTimer -= dt;
        if (comboTimer <= 0.0f) {
            // Combo expired - decay gradually rather than instant break
            if (comboCount > 1) {
                comboCount = std::max(1, comboCount - 1);
                comboTimer = Constants::COMBO_TIMEOUT * 0.5f;
                updateMultiplier();
            } else {
                comboCount = 0;
                multiplier = 1.0f;
            }
        }
    }
}

void ComboSystem::reset() {
    comboCount = 0;
    multiplier = 1.0f;
    comboTimer = 0.0f;
    nearMissCount = 0;
    totalDodges = 0;
    newMilestone = false;
    milestoneValue = 0;
}

int ComboSystem::calculateBonus(int basePoints) const {
    return static_cast<int>(basePoints * multiplier);
}

void ComboSystem::updateMultiplier() {
    // Multiplier grows with combo count, capped at MAX_COMBO_MULTIPLIER
    // Formula: 1.0 + (combo - 1) * 0.2, capped at 10.0
    if (comboCount <= 1) {
        multiplier = 1.0f;
    } else {
        multiplier = std::min(
            static_cast<float>(Constants::MAX_COMBO_MULTIPLIER),
            1.0f + (comboCount - 1) * 0.2f
        );
    }
}

void ComboSystem::checkMilestone() {
    // Check for combo milestones
    static const int milestones[] = {5, 10, 15, 20, 30, 50, 100};
    
    for (int milestone : milestones) {
        if (comboCount == milestone) {
            newMilestone = true;
            milestoneValue = milestone;
            return;
        }
    }
}
