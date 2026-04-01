#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <functional>
#include "GameState.h"
#include "Constants.h"

// Forward declarations
class AssetManager;
class InputManager;
class SaveSystem;

// A single menu item with text and optional callback
struct MenuItem {
    std::string text;
    bool enabled = true;
    bool selected = false;
    std::function<void()> onSelect;
};

// Complete menu system handling all non-gameplay screens.
// Features animated backgrounds, smooth transitions, and modular menu building.
class MenuSystem {
public:
    MenuSystem(sf::RenderWindow& window, AssetManager* assets);

    // Process input for current menu state
    void handleInput(const InputManager& input, GameState& currentState);

    // Update animations
    void update(float dt);

    // Unified render function
    void render(sf::RenderWindow& window, sf::Font& font, GameState state, SaveSystem* saveSystem);

    // Render specific menus (called from Game)
    void renderMainMenu(sf::RenderWindow& window, sf::Font& font);
    void renderSettingsMenu(sf::RenderWindow& window, sf::Font& font);
    void renderPauseMenu(sf::RenderWindow& window, sf::Font& font);
    void renderGameOverMenu(sf::RenderWindow& window, sf::Font& font);
    void renderShopMenu(sf::RenderWindow& window, sf::Font& font, int playerCoins);
    void renderAchievementsMenu(sf::RenderWindow& window, sf::Font& font);

    // Navigation methods
    void navigateUp();
    void navigateDown();
    void navigateLeft();
    void navigateRight();
    void select(GameState& currentState, SaveSystem* saveSystem);
    void back(GameState& currentState);

    // Menu state accessors
    MenuSelection getMainMenuSelection() const { return mainMenuSelection; }
    MenuSelection getPauseSelection() const;
    MenuSelection getGameOverSelection() const;
    void resetMenuState();
    void resetPauseSelection();

    // Set game over stats for display
    void setGameOverStats(float score, float bestScore, int coins, int obstaclesDodged,
                          int maxCombo, float distance);

    // Settings values
    float getMasterVolume() const { return masterVolume; }
    float getMusicVolume() const { return musicVolume; }
    float getSfxVolume() const { return sfxVolume; }
    bool isFullscreen() const { return fullscreen; }

    void setMasterVolume(float vol) { masterVolume = vol; }
    void setMusicVolume(float vol) { musicVolume = vol; }
    void setSfxVolume(float vol) { sfxVolume = vol; }

private:
    sf::RenderWindow& windowRef;
    AssetManager* assets;

    // Menu navigation state
    MenuSelection mainMenuSelection;
    SettingsSection settingsSection;
    int selectedIndex;
    int maxMenuItems;

    // Sub-menu selections
    int pauseSelection;      // 0=Resume, 1=Settings, 2=MainMenu
    int settingsSelection;   // 0=Master, 1=Music, 2=SFX, 3=Fullscreen, 4=Back
    int gameOverSelection;   // 0=Retry, 1=MainMenu
    int mainSelection;       // 0=Play, 1=Settings, 2=Achievements, 3=Quit
    int shopSelection;       // Shop item index

    // Settings values
    float masterVolume;
    float musicVolume;
    float sfxVolume;
    bool fullscreen;

    // Animation state
    float pulseTimer;
    float bgScrollOffset;
    float titleBounce;

    // Game over stats
    float goScore;
    float goBestScore;
    int goCoins;
    int goObstaclesDodged;
    int goMaxCombo;
    float goDistance;

    // Drawing utilities
    void drawTitle(sf::RenderWindow& window, sf::Font& font, const std::string& title, float y);
    void drawMenuItem(sf::RenderWindow& window, sf::Font& font, const std::string& text, float y, bool selected, bool enabled = true);
    void drawSlider(sf::RenderWindow& window, sf::Font& font, const std::string& label, float value, float y, bool selected);
    void drawCenteredText(sf::RenderWindow& window, sf::Font& font, const std::string& text, float y, int fontSize, sf::Color color);
    void drawAnimatedBackground(sf::RenderWindow& window);
    void drawDarkOverlay(sf::RenderWindow& window, sf::Uint8 alpha = 180);
};
