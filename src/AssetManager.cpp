#include "AssetManager.h"
#include "AssetKeys.h"
#include <iostream>

void AssetManager::loadAll() {
    std::cout << "[AssetManager] Loading all assets..." << std::endl;

    // === Active Assets (Runner Runtime) ===
    
    // Player spritesheet
    loadTexture(AssetKeys::PLAYER_SHEET, "assets/textures/student_runner.png");

    // Obstacle textures
    loadTexture(AssetKeys::OBSTACLE_CHAIR, "assets/textures/obstacle_chair.png");
    loadTexture(AssetKeys::OBSTACLE_BENCH, "assets/textures/obstacle_bench.png");
    loadTexture(AssetKeys::OBSTACLE_BOOK, "assets/textures/obstacle_book.png");

    // Parallax background layers
    loadTexture(AssetKeys::BG_FAR, "assets/textures/bg_far.png");
    loadTexture(AssetKeys::BG_MID, "assets/textures/bg_mid.png");
    loadTexture(AssetKeys::BG_NEAR, "assets/textures/bg_near.png");

    // Set parallax textures to repeat
    textures_[AssetKeys::BG_FAR].setRepeated(true);
    textures_[AssetKeys::BG_MID].setRepeated(true);
    textures_[AssetKeys::BG_NEAR].setRepeated(true);

    // Fonts
    loadFont(AssetKeys::MAIN_FONT, "assets/fonts/main.ttf");

    // === Dormant Assets (Platformer Infrastructure) ===
    // These are loaded but not actively used by the runner runtime.
    // They support the dormant TileMap/TmxLoader/Remnant systems.
    
    loadTexture(AssetKeys::PLAYER_LEGACY, "assets/textures/player.png");
    loadTexture(AssetKeys::REMNANT, "assets/textures/remnant.png");
    loadTexture(AssetKeys::COLLEGE_TILESET, "assets/tilesets/college_tileset.png");

    std::cout << "[AssetManager] Loaded " << textures_.size() << " textures, "
              << fonts_.size() << " fonts." << std::endl;
}

void AssetManager::loadTexture(const std::string& key, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        throw AssetLoadException(key, path);
    }
    textures_.emplace(key, std::move(texture));
}

void AssetManager::loadFont(const std::string& key, const std::string& path) {
    sf::Font font;
    if (!font.loadFromFile(path)) {
        throw AssetLoadException(key, path);
    }
    fonts_.emplace(key, std::move(font));
}

// === Template Specializations ===

template<>
sf::Texture& AssetManager::get<sf::Texture>(const std::string& key) {
    auto it = textures_.find(key);
    if (it == textures_.end()) {
        throw AssetLoadException("Texture not found: " + key);
    }
    return it->second;
}

template<>
sf::Font& AssetManager::get<sf::Font>(const std::string& key) {
    auto it = fonts_.find(key);
    if (it == fonts_.end()) {
        throw AssetLoadException("Font not found: " + key);
    }
    return it->second;
}

template<>
bool AssetManager::has<sf::Texture>(const std::string& key) const {
    return textures_.find(key) != textures_.end();
}

template<>
bool AssetManager::has<sf::Font>(const std::string& key) const {
    return fonts_.find(key) != fonts_.end();
}
