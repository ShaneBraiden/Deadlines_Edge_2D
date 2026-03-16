#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include "PhysicsWorld.h"
#include "Player.h"
#include "Remnant.h"
#include "InputManager.h"
#include "GameState.h"
#include "ResourceManager.h"
#include "TileMap.h"
#include "ParticleSystem.h"
#include "ParallaxBackground.h"
#include "LightingSystem.h"

// Core game class — owns the window, physics world, player, and all entities.
// Runs the main loop with a fixed-timestep accumulator pattern.
//
// Lab requirements covered:
//   - Single-level pointers: PhysicsWorld*, Player*, Entity*
//   - Arrow operator: physics->step(), entity->update()
//   - STL vector: std::vector<Entity*> for polymorphic entity storage
//   - STL iterators: explicit iterator loops over entities
//   - Class templates: ResourceManager<sf::Texture>, ResourceManager<sf::Font>
//   - Exception handling: try/catch around resource preloading

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void startGameFromMenu();
    void applyGameplayView();

    void processEvents();
    void update(float dt);
    void render();

    // Level management
    void loadLevel(int levelIndex);
    void clearLevel();
    void checkLevelTransition();

    // Menu/ending rendering
    void renderMenu();
    void renderPause();
    void renderEnding();
    void renderVignette();

    sf::RenderWindow window;

    // Lab: single-level pointer
    PhysicsWorld* physics;

    // Lab: STL vector of single-level pointers
    std::vector<Entity*> entities;

    // Non-owning pointers
    Player* player;
    std::vector<Remnant*> remnants;

    // Systems
    InputManager inputManager;
    GameState currentState;
    ResourceManager<sf::Texture> textures;
    ResourceManager<sf::Font> fonts;
    TileMap* tileMap;                    // Lab: single-level pointer
    ParticleSystem* particleSystem;     // Lab: single-level pointer
    ParallaxBackground* parallaxBg;    // Lab: single-level pointer
    LightingSystem* lightingSystem;    // Lab: single-level pointer

    // Level tracking
    int currentLevel;
    static const int TOTAL_LEVELS = 3;
    std::string levelFiles[TOTAL_LEVELS];

    // Ending sequence
    float endingTimer;
    float endingFadeAlpha;
    bool endingTriggered;

    // Vignette overlay
    sf::RectangleShape vignetteTop;
    sf::RectangleShape vignetteBottom;
    sf::RectangleShape vignetteLeft;
    sf::RectangleShape vignetteRight;
};
