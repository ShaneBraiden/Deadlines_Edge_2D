#pragma once

#include <SFML/Graphics.hpp>
#include <unordered_map>
#include <string>
#include <stdexcept>

// Custom exception for asset loading failures.
// Extends std::runtime_error so it integrates with existing main.cpp try/catch.
//
// Lab requirements covered:
//   - Exception handling: custom exception class extending std::runtime_error

class AssetLoadException : public std::runtime_error {
public:
    explicit AssetLoadException(const std::string& message)
        : std::runtime_error(message) {}
    
    AssetLoadException(const std::string& assetKey, const std::string& filePath)
        : std::runtime_error("Failed to load asset '" + assetKey + "' from '" + filePath + "'") {}
};

// Centralized asset manager for all game resources.
// All SFML resource loading (textures, fonts) goes through this class.
//
// Lab requirements covered:
//   - Class templates: get<T>() template method
//   - STL containers: std::unordered_map for O(1) asset lookup
//   - Exception handling: throws AssetLoadException on failures

class AssetManager {
public:
    AssetManager() = default;
    ~AssetManager() = default;

    // Non-copyable (SFML resources shouldn't be copied)
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    // Loads all registered assets. Call once at startup.
    // Throws AssetLoadException if any asset fails to load.
    void loadAll();

    // Template getter for type-safe asset retrieval.
    // Throws AssetLoadException if asset not found.
    //
    // Lab requirements covered:
    //   - Templates: function template with explicit specializations
    //
    // Usage:
    //   sf::Texture& tex = assets.get<sf::Texture>(AssetKeys::PLAYER_SHEET);
    //   sf::Font& font = assets.get<sf::Font>(AssetKeys::MAIN_FONT);
    template<typename T>
    T& get(const std::string& key);

    // Check if an asset is loaded
    template<typename T>
    bool has(const std::string& key) const;

    // Get number of loaded assets by type
    std::size_t textureCount() const { return textures_.size(); }
    std::size_t fontCount() const { return fonts_.size(); }

private:
    // Internal storage
    // Lab: STL containers — unordered_map for O(1) lookup
    std::unordered_map<std::string, sf::Texture> textures_;
    std::unordered_map<std::string, sf::Font>    fonts_;

    // Helper to load a single texture
    void loadTexture(const std::string& key, const std::string& path);

    // Helper to load a single font
    void loadFont(const std::string& key, const std::string& path);
};

// Template specializations declared here, defined in .cpp
template<>
sf::Texture& AssetManager::get<sf::Texture>(const std::string& key);

template<>
sf::Font& AssetManager::get<sf::Font>(const std::string& key);

template<>
bool AssetManager::has<sf::Texture>(const std::string& key) const;

template<>
bool AssetManager::has<sf::Font>(const std::string& key) const;
