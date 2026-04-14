#include "SaveSystem.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

SaveSystem::SaveSystem()
    : savePath("deadlines_edge_save.dat")
{
    load();  // Try to load existing save
}

bool SaveSystem::load() {
    std::ifstream file(savePath);
    if (!file.is_open()) {
        std::cout << "[SaveSystem] No save file found, starting fresh." << std::endl;
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    if (!deserialize(buffer.str())) {
        std::cout << "[SaveSystem] Failed to parse save file, resetting." << std::endl;
        reset();
        return false;
    }

    std::cout << "[SaveSystem] Loaded save: " << data.totalRuns << " runs, "
              << data.totalCoins << " coins, best: " << data.bestScore << std::endl;
    return true;
}

bool SaveSystem::save() {
    std::ofstream file(savePath);
    if (!file.is_open()) {
        std::cerr << "[SaveSystem] Failed to open save file for writing." << std::endl;
        return false;
    }

    file << serialize();
    file.close();

    std::cout << "[SaveSystem] Saved progress." << std::endl;
    return true;
}

void SaveSystem::updateBestScore(float score) {
    if (score > data.bestScore) {
        data.bestScore = score;
    }
}

void SaveSystem::addCoins(int amount) {
    data.totalCoins += amount;
}

bool SaveSystem::spendCoins(int amount) {
    if (data.totalCoins < amount) return false;
    data.totalCoins -= amount;
    return true;
}

void SaveSystem::incrementRuns() {
    data.totalRuns++;
}

void SaveSystem::addPlayTime(float seconds) {
    data.totalPlayTime += seconds;
}

void SaveSystem::addObstaclesDodged(int count) {
    data.totalObstaclesDodged += count;
}

void SaveSystem::updateHighestCombo(int combo) {
    if (combo > data.highestCombo) {
        data.highestCombo = combo;
    }
}

void SaveSystem::addNearMisses(int count) {
    data.totalNearMisses += count;
}

bool SaveSystem::isAchievementUnlocked(const std::string& id) const {
    return data.unlockedAchievements.find(id) != data.unlockedAchievements.end();
}

void SaveSystem::unlockAchievement(const std::string& id) {
    data.unlockedAchievements.insert(id);
}

std::vector<std::string> SaveSystem::getUnlockedAchievements() const {
    return std::vector<std::string>(data.unlockedAchievements.begin(),
                                     data.unlockedAchievements.end());
}

void SaveSystem::reset() {
    data = PlayerData();
}

std::string SaveSystem::serialize() const {
    std::stringstream ss;

    // Simple key=value format
    ss << "bestScore=" << data.bestScore << "\n";
    ss << "totalRuns=" << data.totalRuns << "\n";
    ss << "totalObstaclesDodged=" << data.totalObstaclesDodged << "\n";
    ss << "totalPlayTime=" << data.totalPlayTime << "\n";
    ss << "totalCoins=" << data.totalCoins << "\n";
    ss << "highestCombo=" << data.highestCombo << "\n";
    ss << "totalNearMisses=" << data.totalNearMisses << "\n";
    ss << "extraLives=" << data.extraLives << "\n";
    ss << "doubleCoinsUpgrade=" << (data.doubleCoinsUpgrade ? 1 : 0) << "\n";
    ss << "shieldStartUpgrade=" << (data.shieldStartUpgrade ? 1 : 0) << "\n";

    // Achievements as comma-separated list
    ss << "achievements=";
    bool first = true;
    for (const auto& ach : data.unlockedAchievements) {
        if (!first) ss << ",";
        ss << ach;
        first = false;
    }
    ss << "\n";

    return ss.str();
}

bool SaveSystem::deserialize(const std::string& content) {
    std::istringstream stream(content);
    std::string line;

    data = PlayerData();  // Start fresh

    while (std::getline(stream, line)) {
        size_t eqPos = line.find('=');
        if (eqPos == std::string::npos) continue;

        std::string key = line.substr(0, eqPos);
        std::string value = line.substr(eqPos + 1);

        try {
            if (key == "bestScore") {
                data.bestScore = std::stof(value);
            } else if (key == "totalRuns") {
                data.totalRuns = std::stoi(value);
            } else if (key == "totalObstaclesDodged") {
                data.totalObstaclesDodged = std::stoi(value);
            } else if (key == "totalPlayTime") {
                data.totalPlayTime = std::stof(value);
            } else if (key == "totalCoins") {
                data.totalCoins = std::stoi(value);
            } else if (key == "highestCombo") {
                data.highestCombo = std::stoi(value);
            } else if (key == "totalNearMisses") {
                data.totalNearMisses = std::stoi(value);
            } else if (key == "extraLives") {
                data.extraLives = std::stoi(value);
            } else if (key == "doubleCoinsUpgrade") {
                data.doubleCoinsUpgrade = (value == "1");
            } else if (key == "shieldStartUpgrade") {
                data.shieldStartUpgrade = (value == "1");
            } else if (key == "achievements") {
                // Parse comma-separated achievements
                std::istringstream achStream(value);
                std::string ach;
                while (std::getline(achStream, ach, ',')) {
                    if (!ach.empty()) {
                        data.unlockedAchievements.insert(ach);
                    }
                }
            }
        } catch (...) {
            // Ignore parse errors for individual fields
        }
    }

    return true;
}
