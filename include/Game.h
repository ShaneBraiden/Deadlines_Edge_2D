#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <list>
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
    void updateGameplayMusic();

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
    void checkParkourLanding();
    void updateProfessors(float dt);
    void updateAssignments(float dt);
    void checkAssignmentPlayerCollision();
    void checkAssignmentBulletCollision();

    // Obstacle types
    enum class ObstacleType {
        Chair,
        Bench,
        Book,
        Professor,  // New: slow-moving, must jump
        ExamStack   // New: tall stack, must duck
    };

    // Shooting system
    void handleShooting();
    void updateProjectiles(float dt);
    void checkProjectileCollisions();

    struct RunnerObstacle;  // forward declare so Assignment can point back

    // Assignment projectile thrown by the professor
    struct Assignment {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float rotation;
        float rotationSpeed;
        int hitPoints;              // 2 hits to destroy with pen
        bool active;
        float animTimer;
        int currentFrame;
        RunnerObstacle* owner;      // Which professor threw this (nullptr = none)
        static constexpr int MAX_FRAME = 4;
        static constexpr float FRAME_TIME = 0.12f;
    };

    struct RunnerObstacle {
        sf::Sprite sprite;
        float speed;
        bool passed;
        ObstacleType type;
        float animTimer;        // General animation timer
        int hitPoints;          // Health: destroyed when reaches 0
        b2Body* physBody;       // Kinematic body for bench/chair platforms (nullptr otherwise)
        bool playerOnTop;       // True when player is standing on this obstacle

        // Professor-specific
        int   profFrame;             // Current spritesheet frame (0-7)
        float profFrameTimer;        // Time since last frame change
        float profThrowTimer;        // Countdown until next assignment throw
        int   assignmentsThrown;     // Total thrown so far (max 3)
        int   assignmentsDestroyed;  // How many of its own assignments were destroyed
        bool  dying;                 // Playing disappear animation
        float dyingTimer;            // 0 → DYING_DURATION then erased
        static constexpr int   PROF_COLS              = 4;
        static constexpr int   PROF_ROWS              = 2;
        static constexpr int   PROF_FRAME_COUNT       = 8;
        static constexpr float PROF_FRAME_TIME        = 0.10f;
        static constexpr float PROF_THROW_DELAY       = 2.2f;
        static constexpr int   PROF_MAX_THROWS        = 1;
        static constexpr float DYING_DURATION         = 0.55f;
        static constexpr int   DEFAULT_HIT_POINTS     = 2;
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
    float nextAmmoDropDistance;

    // Vignette overlay
    sf::RectangleShape vignetteTop;
    sf::RectangleShape vignetteBottom;
    sf::RectangleShape vignetteLeft;
    sf::RectangleShape vignetteRight;

    // Assignment projectiles thrown by professors
    std::list<Assignment> assignments;

    // Main gameplay music (assets/audio/main.mp3)
    sf::Music gameplayMusic;

    // Bullet/pen sound effect
    sf::SoundBuffer bulletSoundBuffer;
    sf::Sound       bulletSound;
};
