#pragma once

#include <SFML/Graphics.hpp>
#include <vector>
#include <string>
#include <random>
#include "PhysicsWorld.h"
#include "Player.h"
#include "InputManager.h"
#include "GameState.h"
#include "AssetManager.h"
#include "ParticleSystem.h"
#include "ParallaxBackground.h"
#include "LightingSystem.h"

// Core game class — owns the window, physics world, player, and all entities.
// Runs the main loop with a fixed-timestep accumulator pattern.
//
// Lab requirements covered:
//   - Single-level pointers: PhysicsWorld*, Player*, Entity*, AssetManager*
//   - Arrow operator: physics->step(), entity->update(), assets->get<T>()
//   - STL vector: std::vector<Entity*> for polymorphic entity storage
//   - STL iterators: explicit iterator loops over entities
//   - Class templates: AssetManager::get<T>() for type-safe asset retrieval
//   - Exception handling: AssetLoadException propagates to main.cpp

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void startGameFromMenu();
    void resetRun();
    void spawnObstacle();
    void updateObstacles(float dt);
    bool checkObstacleCollision() const;

    void processEvents();
    void update(float dt);
    void render();

    // Menu/ending rendering
    void renderMenu();
    void renderPause();
    void renderGameOver();
    void renderVignette();

    struct RunnerObstacle {
        sf::Sprite sprite;
        float speed;
        bool passed;
    };

    sf::RenderWindow window;

    // Lab: single-level pointer
    PhysicsWorld* physics;

    // Lab: STL vector of single-level pointers
    std::vector<Entity*> entities;

    // Non-owning pointers
    Player* player;
    b2Body* groundBody;

    // Systems
    InputManager inputManager;
    GameState currentState;
    AssetManager* assets;               // Lab: single-level pointer, template get<T>()
    ParticleSystem* particleSystem;     // Lab: single-level pointer
    ParallaxBackground* parallaxBg;     // Lab: single-level pointer
    LightingSystem* lightingSystem;     // Lab: single-level pointer

    // Runner systems
    std::vector<RunnerObstacle> obstacles;
    float obstacleSpawnTimer;
    float nextObstacleSpawnDelay;
    float obstacleBaseSpeed;
    float score;
    float bestScore;
    bool gameOver;
    std::mt19937 rng;

    float laneX;
    float groundY;

    // Vignette overlay
    sf::RectangleShape vignetteTop;
    sf::RectangleShape vignetteBottom;
    sf::RectangleShape vignetteLeft;
    sf::RectangleShape vignetteRight;
};
