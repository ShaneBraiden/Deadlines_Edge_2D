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
#include "MenuSystem.h"
#include "HUD.h"
#include "ComboSystem.h"
#include "PowerUpManager.h"
#include "CoinManager.h"
#include "ScreenEffects.h"
#include "SaveSystem.h"
#include "AchievementSystem.h"
#include "ProjectileManager.h"

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
    bool checkObstacleCollision();
    bool checkNearMiss() const;

    void processEvents();
    void update(float dt);
    void render();

    // Menu/ending rendering
    void renderMenu();
    void renderPause();
    void renderGameOver();
    void renderVignette();

    // New systems updates
    void updatePowerUps(float dt);
    void updateCoins(float dt);
    void updateCombo(float dt);
    void checkAchievements();
    void handleGameOver();

    // Obstacle types
    enum class ObstacleType {
        Chair,
        Bench,
        Book,
        Professor,  // New: slow-moving, must jump
        ExamStack,  // New: tall stack, must duck
        CoffeeCart  // New: wide, must jump high
    };

    // Shooting system
    void handleShooting();
    void updateProjectiles(float dt);
    void checkProjectileCollisions();

    struct RunnerObstacle {
        sf::Sprite sprite;
        float speed;
        bool passed;
        ObstacleType type;
        float animTimer;  // For animated obstacles
        int hitPoints;    // Health: destroyed when reaches 0
        static constexpr int DEFAULT_HIT_POINTS = 2;  // Takes 2 hits to destroy
    };

    sf::RenderWindow window;

    // Lab: single-level pointer
    PhysicsWorld* physics;

    // Lab: STL vector of single-level pointers
    std::vector<Entity*> entities;

    // Non-owning pointers
    Player* player;
    b2Body* groundBody;

    // Core systems
    InputManager inputManager;
    GameState currentState;
    GameState previousState;  // For transitions
    AssetManager* assets;               // Lab: single-level pointer, template get<T>()
    ParticleSystem* particleSystem;     // Lab: single-level pointer
    ParallaxBackground* parallaxBg;     // Lab: single-level pointer
    LightingSystem* lightingSystem;     // Lab: single-level pointer

    // New UI systems
    MenuSystem* menuSystem;
    HUD* hud;
    ScreenEffects* screenEffects;

    // New gameplay systems
    ComboSystem* comboSystem;
    PowerUpManager* powerUpManager;
    CoinManager* coinManager;
    SaveSystem* saveSystem;
    AchievementSystem* achievementSystem;
    ProjectileManager* projectileManager;

    // Runner state
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

    // Run statistics
    float runTime;
    int obstaclesDodged;
    int coinsThisRun;
    float distanceTraveled;

    // Vignette overlay
    sf::RectangleShape vignetteTop;
    sf::RectangleShape vignetteBottom;
    sf::RectangleShape vignetteLeft;
    sf::RectangleShape vignetteRight;
};
