#include "Game.h"
#include "Constants.h"
#include "AssetKeys.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <iostream>

namespace {

float randomRange(std::mt19937& rng, float minV, float maxV) {
    std::uniform_real_distribution<float> dist(minV, maxV);
    return dist(rng);
}

} // namespace

Game::Game()
    : window(sf::VideoMode::getDesktopMode(), "Deadline's Edge: Campus Sprint", sf::Style::Fullscreen)
    , physics(nullptr)
    , player(nullptr)
    , groundBody(nullptr)
    , particleSystem(nullptr)
    , parallaxBg(nullptr)
    , lightingSystem(nullptr)
    , menuSystem(nullptr)
    , hud(nullptr)
    , screenEffects(nullptr)
    , comboSystem(nullptr)
    , powerUpManager(nullptr)
    , coinManager(nullptr)
    , saveSystem(nullptr)
    , achievementSystem(nullptr)
    , projectileManager(nullptr)
    , currentState(GameState::Menu)
    , previousState(GameState::Menu)
    , assets(nullptr)
    , obstacleSpawnTimer(0.0f)
    , nextObstacleSpawnDelay(1.2f)
    , obstacleBaseSpeed(344.0f)
    , score(0.0f)
    , bestScore(0.0f)
    , gameOver(false)
    , rng(static_cast<unsigned int>(std::time(nullptr)))
    , laneX(0.0f)
    , groundY(0.0f)
    , runTime(0.0f)
    , obstaclesDodged(0)
    , coinsThisRun(0)
    , distanceTraveled(0.0f)
    , nextAmmoDropDistance(0.0f)
    , nextProfId(0)
    , extraLivesRemaining(0)
    , playerWasOnGround(true)
{
    window.setFramerateLimit(60);

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Initialize save system first
    saveSystem = new SaveSystem();
    bestScore = saveSystem->getData().bestScore;

    // Lab: single-level pointer + exception handling
    assets = new AssetManager();
    assets->loadAll();

    physics = new PhysicsWorld(winW, winH);
    particleSystem = new ParticleSystem(winW, winH);
    parallaxBg = new ParallaxBackground();

    // Particle system is fully procedural - no textures needed
    particleSystem->setTextures(nullptr, nullptr, nullptr);

    // Lab: arrow operator + template method call
    parallaxBg->addLayer(assets->get<sf::Texture>(AssetKeys::BG_FAR), 0.10f);
    parallaxBg->addLayer(assets->get<sf::Texture>(AssetKeys::BG_MID), 0.28f);
    parallaxBg->addLayer(assets->get<sf::Texture>(AssetKeys::BG_NEAR), 0.54f);

    lightingSystem = new LightingSystem(
        static_cast<unsigned int>(winW),
        static_cast<unsigned int>(winH)
    );
    lightingSystem->setAmbientLevel(190);

    // Initialize new UI systems
    menuSystem = new MenuSystem(window, assets);
    hud = new HUD(window, assets);
    screenEffects = new ScreenEffects(window);

    // Initialize gameplay systems
    groundY = winH - 160.0f;
    comboSystem = new ComboSystem();
    powerUpManager = new PowerUpManager(winW, groundY);
    coinManager = new CoinManager(winW, groundY);
    achievementSystem = new AchievementSystem(saveSystem);
    projectileManager = new ProjectileManager();
    
    // Set projectile texture (bullet only, muzzle flash is procedural)
    projectileManager->setTextures(
        &assets->get<sf::Texture>(AssetKeys::BULLET),
        nullptr
    );

    // Setup vignette
    float vigSize = 130.0f;
    vignetteTop.setSize(sf::Vector2f(winW, vigSize));
    vignetteTop.setPosition(0.0f, 0.0f);
    vignetteTop.setFillColor(sf::Color(0, 0, 0, 95));

    vignetteBottom.setSize(sf::Vector2f(winW, vigSize));
    vignetteBottom.setPosition(0.0f, winH - vigSize);
    vignetteBottom.setFillColor(sf::Color(0, 0, 0, 115));

    vignetteLeft.setSize(sf::Vector2f(vigSize, winH));
    vignetteLeft.setPosition(0.0f, 0.0f);
    vignetteLeft.setFillColor(sf::Color(0, 0, 0, 70));

    vignetteRight.setSize(sf::Vector2f(vigSize, winH));
    vignetteRight.setPosition(winW - vigSize, 0.0f);
    vignetteRight.setFillColor(sf::Color(0, 0, 0, 70));

    // Gameplay music (loops during play)
    if (!gameplayMusic.openFromFile("assets/audio/main.mp3")) {
        std::cerr << "Warning: Failed to load main music\n";
    } else {
        gameplayMusic.setLoop(true);
        gameplayMusic.setVolume(60.0f);
    }

    // Menu music (loops on menus)
    if (!menuMusic.openFromFile("assets/audio/menu.mp3")) {
        std::cerr << "Warning: Failed to load menu music\n";
    } else {
        menuMusic.setLoop(true);
        menuMusic.setVolume(50.0f);
        menuMusic.play();
    }

    // SFX — bullet/pen
    if (bulletSoundBuffer.loadFromFile("assets/audio/bullet.mp3")) {
        bulletSound.setBuffer(bulletSoundBuffer);
        bulletSound.setVolume(75.0f);
    }

    // SFX — coin pickup
    if (coinSoundBuffer.loadFromFile("assets/audio/coin.mp3")) {
        coinSound.setBuffer(coinSoundBuffer);
        coinSound.setVolume(70.0f);
    }

    // SFX — power-up collected
    if (powerupSoundBuffer.loadFromFile("assets/audio/powerup.mp3")) {
        powerupSound.setBuffer(powerupSoundBuffer);
        powerupSound.setVolume(80.0f);
    }

    // SFX — game lost
    if (gameLostSoundBuffer.loadFromFile("assets/audio/game_lost.mp3")) {
        gameLostSound.setBuffer(gameLostSoundBuffer);
        gameLostSound.setVolume(85.0f);
    }

    // SFX — jump
    if (jumpSoundBuffer.loadFromFile("assets/audio/jump.mp3")) {
        jumpSound.setBuffer(jumpSoundBuffer);
        jumpSound.setVolume(75.0f);
    }

    // SFX — parkour land on bench/chair
    if (landSoundBuffer.loadFromFile("assets/audio/land.mp3")) {
        landSound.setBuffer(landSoundBuffer);
        landSound.setVolume(80.0f);
    }
}

Game::~Game() {
    // Save progress before cleanup
    if (saveSystem) {
        saveSystem->save();
    }

    for (auto* entity : entities) {
        delete entity;
    }
    entities.clear();

    delete achievementSystem;
    achievementSystem = nullptr;

    delete projectileManager;
    projectileManager = nullptr;

    delete saveSystem;
    saveSystem = nullptr;

    delete coinManager;
    coinManager = nullptr;

    delete powerUpManager;
    powerUpManager = nullptr;

    delete comboSystem;
    comboSystem = nullptr;

    delete screenEffects;
    screenEffects = nullptr;

    delete hud;
    hud = nullptr;

    delete menuSystem;
    menuSystem = nullptr;

    delete lightingSystem;
    lightingSystem = nullptr;

    delete parallaxBg;
    parallaxBg = nullptr;

    delete particleSystem;
    particleSystem = nullptr;

    delete physics;
    physics = nullptr;

    delete assets;
    assets = nullptr;
}

void Game::startGameFromMenu() {
    resetRun();
    currentState = GameState::Play;
}

void Game::resetRun() {
    for (auto* entity : entities) {
        delete entity;
    }
    entities.clear();

    // Destroy any kinematic obstacle bodies before clearing the list
    // (physics world is about to be recreated, but clean up explicitly for safety)
    for (auto& obs : obstacles) {
        obs.physBody = nullptr;  // Old physics world owns the memory; it's deleted below
    }
    obstacles.clear();
    player = nullptr;
    groundBody = nullptr;

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    delete physics;
    physics = new PhysicsWorld(winW, winH);

    laneX = winW * 0.23f;
    groundY = winH - 160.0f;  // Higher ground line for bigger player

    // Create a thick ground body so foot sensor reliably touches it
    const float groundHalfHeightMeters = 1.0f;
    float groundTopScreenY = groundY;
    float groundCenterScreenY = groundTopScreenY + groundHalfHeightMeters * Constants::PPM;
    b2Vec2 groundWorldPos = physics->toWorld(sf::Vector2f(winW * 0.5f, groundCenterScreenY));
    groundBody = physics->createStaticBody(
        groundWorldPos.x,
        groundWorldPos.y,
        physics->getWorldWidth(),
        groundHalfHeightMeters
    );

    // Lab: arrow operator + template method call
    sf::Texture& playerTex = assets->get<sf::Texture>(AssetKeys::PLAYER_SHEET);
    float playerHalfHeightPixels = Constants::PLAYER_HEIGHT * Constants::PPM * 0.5f;
    float playerCenterScreenY = groundY - playerHalfHeightPixels - 5.0f;
    b2Vec2 playerSpawn = physics->toWorld(sf::Vector2f(laneX, playerCenterScreenY));
    player = new Player(physics, playerSpawn.x, playerSpawn.y, playerTex);
    player->setRunnerMode(true);

    // Apply shield start upgrade if purchased
    if (saveSystem && saveSystem->getData().shieldStartUpgrade) {
        powerUpManager->activatePowerUp(PowerUpType::Shield);
    }

    entities.push_back(player);

    // Reset runner state
    obstacleSpawnTimer = 0.0f;
    nextObstacleSpawnDelay = 1.32f;   // 1.1 * 1.2 — matches reduced spawn rate
    obstacleBaseSpeed = 344.0f;       // 430 * 0.8 — 20% slower start
    score = 0.0f;
    gameOver = false;

    // Reset run statistics
    runTime = 0.0f;
    obstaclesDodged = 0;
    coinsThisRun = 0;
    distanceTraveled = 0.0f;
    nextAmmoDropDistance = randomRange(
        rng,
        Constants::PROJECTILE_AMMO_DROP_DISTANCE_MIN,
        Constants::PROJECTILE_AMMO_DROP_DISTANCE_MAX
    );

    // Reset systems
    comboSystem->reset();
    powerUpManager->reset();
    coinManager->reset();
    screenEffects->reset();
    hud->clearPowerUps();
    projectileManager->reset();
    assignments.clear();
    nextProfId = 0;
    playerWasOnGround = true;

    // Apply extra lives upgrade
    extraLivesRemaining = saveSystem ? saveSystem->getData().extraLives : 0;

    // Increment run count
    if (saveSystem) {
        saveSystem->incrementRuns();
    }
}

void Game::spawnObstacle() {
    std::uniform_int_distribution<int> typeDist(0, 99);
    int roll = typeDist(rng);

    RunnerObstacle obstacle;
    obstacle.speed = obstacleBaseSpeed;
    obstacle.passed = false;
    obstacle.animTimer = 0.0f;
    obstacle.hitPoints = RunnerObstacle::DEFAULT_HIT_POINTS;
    obstacle.physBody = nullptr;
    obstacle.playerOnTop = false;
    obstacle.profId          = nextProfId++;
    obstacle.profFrame       = 0;
    obstacle.profFrameTimer  = 0.0f;
    obstacle.profThrowTimer  = RunnerObstacle::PROF_THROW_DELAY;
    obstacle.assignmentsThrown = 0;
    obstacle.dying           = false;
    obstacle.dyingTimer      = 0.0f;

    float spawnY = groundY;

    // Minimum gap between obstacles, expanded by 2.5x as requested.
    constexpr float MIN_OBSTACLE_SPACING = 260.0f;
    constexpr float OBSTACLE_GAP = MIN_OBSTACLE_SPACING * 2.5f;

    // Start from the right edge of the last obstacle so spacing is deterministic.
    float spawnX = static_cast<float>(window.getSize().x) + 60.0f;

    // Lab: arrow operator + template method call
    if (roll < 35) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_CHAIR));
        obstacle.sprite.setScale(0.22f, 0.22f);
        obstacle.type = ObstacleType::Chair;
    }
    else if (roll < 60) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_BENCH));
        obstacle.sprite.setScale(0.28f, 0.28f);
        obstacle.type = ObstacleType::Bench;
    }
    else if (roll < 80) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_BOOK));
        obstacle.sprite.setScale(0.15f, 0.15f);
        spawnY = groundY - 220.0f;  // Flying book — raised higher, must duck or jump under
        obstacle.type = ObstacleType::Book;
    }
    else {
        // Professor — animated spritesheet (2 rows x 4 cols = 8 frames)
        sf::Texture& profTex = assets->get<sf::Texture>(AssetKeys::OBSTACLE_PROFESSOR);
        obstacle.sprite.setTexture(profTex);
        sf::Vector2u ts = profTex.getSize();
        int fw = static_cast<int>(ts.x) / RunnerObstacle::PROF_COLS;
        int fh = static_cast<int>(ts.y) / RunnerObstacle::PROF_ROWS;
        obstacle.sprite.setTextureRect(sf::IntRect(0, 0, fw, fh));
        obstacle.sprite.setScale(0.55f, 0.55f);
        obstacle.type = ObstacleType::Professor;
    }

    sf::FloatRect local = obstacle.sprite.getLocalBounds();
    float scaledWidth = local.width * std::abs(obstacle.sprite.getScale().x);

    if (!obstacles.empty()) {
        const auto& previousObstacle = obstacles.back();
        sf::FloatRect previousBounds = previousObstacle.sprite.getGlobalBounds();
        float previousBackEdge = previousBounds.left + previousBounds.width;
        spawnX = previousBackEdge + OBSTACLE_GAP + (scaledWidth * 0.5f);
    }

    obstacle.sprite.setOrigin(local.width * 0.5f, local.height);
    obstacle.sprite.setPosition(spawnX, spawnY);

    // Bench and chair become standable platforms via a kinematic Box2D body.
    // The body top surface is flush with the sprite top so the player can land on it.
    if (obstacle.type == ObstacleType::Chair || obstacle.type == ObstacleType::Bench) {
        sf::FloatRect gb = obstacle.sprite.getGlobalBounds();
        float halfW = physics->toMeters(gb.width  * 0.5f);
        float halfH = physics->toMeters(gb.height * 0.5f);
        // Sprite origin is bottom-centre, so centre Y in screen = spawnY - gb.height*0.5
        sf::Vector2f centerScreen(spawnX, spawnY - gb.height * 0.5f);
        b2Vec2 worldCenter = physics->toWorld(centerScreen);
        obstacle.physBody = physics->createKinematicBody(worldCenter.x, worldCenter.y, halfW, halfH);
    }

    obstacles.push_back(obstacle);
}

void Game::updateObstacles(float dt) {
    float timeScale = powerUpManager->getSlowMoFactor();

    for (auto& obstacle : obstacles) {
        // Apply slow-mo to obstacle movement
        obstacle.sprite.move(-obstacle.speed * dt * timeScale, 0.0f);

        // Update animation timer for animated obstacles
        obstacle.animTimer += dt;

        // Bobbing animation for flying books — raised higher
        if (obstacle.type == ObstacleType::Book) {
            float bob = std::sin(obstacle.animTimer * 4.5f) * 10.0f;
            sf::Vector2f pos = obstacle.sprite.getPosition();
            obstacle.sprite.setPosition(pos.x, groundY - 195.0f + bob);
        }

        // Sync kinematic physics body to the sprite position each frame
        if (obstacle.physBody) {
            sf::FloatRect gb = obstacle.sprite.getGlobalBounds();
            sf::Vector2f centerScreen(
                obstacle.sprite.getPosition().x,
                obstacle.sprite.getPosition().y - gb.height * 0.5f
            );
            b2Vec2 worldCenter = physics->toWorld(centerScreen);
            float screenVelX = -obstacle.speed * timeScale;
            obstacle.physBody->SetLinearVelocity(
                b2Vec2(physics->toMeters(screenVelX), 0.0f));
            obstacle.physBody->SetTransform(worldCenter, 0.0f);
        }

        // Check if passed player lane
        if (!obstacle.passed && obstacle.sprite.getPosition().x < laneX) {
            obstacle.passed = true;
            obstaclesDodged++;

            int baseBonus = 20;
            float scoreMultiplier = powerUpManager->getScoreMultiplier();
            int finalBonus = comboSystem->calculateBonus(static_cast<int>(baseBonus * scoreMultiplier));
            score += finalBonus;
            comboSystem->registerDodge();
            hud->triggerScorePopup("+" + std::to_string(finalBonus),
                sf::Vector2f(laneX + 50.0f, groundY - 100.0f));
        }
    }

    // Destroy physics bodies of off-screen obstacles before erasing them
    for (auto& obstacle : obstacles) {
        sf::FloatRect bounds = obstacle.sprite.getGlobalBounds();
        if (bounds.left + bounds.width < -30.0f && obstacle.physBody) {
            physics->getWorld()->DestroyBody(obstacle.physBody);
            obstacle.physBody = nullptr;
        }
    }

    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(), [](const RunnerObstacle& o) {
            sf::FloatRect bounds = o.sprite.getGlobalBounds();
            return bounds.left + bounds.width < -30.0f;
        }),
        obstacles.end()
    );
}

bool Game::checkObstacleCollision() {
    if (!player || player->isInvulnerable()) {
        return false;
    }

    sf::FloatRect playerBounds = player->getBoundsScreen();
    playerBounds.left += playerBounds.width * 0.28f;
    playerBounds.top += playerBounds.height * 0.14f;
    playerBounds.width *= 0.46f;
    playerBounds.height *= 0.80f;

    float playerBottom = playerBounds.top + playerBounds.height;

    for (const auto& obstacle : obstacles) {
        // Already scrolled behind the player — never count as a hit
        if (obstacle.passed) continue;

        sf::FloatRect obstacleBounds = obstacle.sprite.getGlobalBounds();
        obstacleBounds.left += obstacleBounds.width * 0.12f;
        obstacleBounds.top  += obstacleBounds.height * 0.12f;
        obstacleBounds.width  *= 0.76f;
        obstacleBounds.height *= 0.82f;

        // Player is cleanly above this obstacle — a good jump clears it
        if (!player->isOnGround() && playerBottom <= obstacleBounds.top) continue;

        // Bench / chair are platforms: only register a hit when the player runs into
        // the SIDE — not when they are standing on top or are above the top surface.
        if (obstacle.physBody) {
            // Allow up to 20px below the obstacle's visual top before counting as a side hit
            if (playerBottom < obstacleBounds.top + 20.0f) continue;
        }

        if (playerBounds.intersects(obstacleBounds)) {
            return true;
        }
    }

    return false;
}

bool Game::checkNearMiss() const {
    if (!player) return false;

    sf::FloatRect playerBounds = player->getBoundsScreen();
    // Expand bounds for near-miss detection
    playerBounds.left -= Constants::NEAR_MISS_THRESHOLD;
    playerBounds.top -= Constants::NEAR_MISS_THRESHOLD;
    playerBounds.width += Constants::NEAR_MISS_THRESHOLD * 2;
    playerBounds.height += Constants::NEAR_MISS_THRESHOLD * 2;

    sf::FloatRect tightPlayerBounds = player->getBoundsScreen();
    tightPlayerBounds.left += tightPlayerBounds.width * 0.28f;
    tightPlayerBounds.top += tightPlayerBounds.height * 0.14f;
    tightPlayerBounds.width *= 0.46f;
    tightPlayerBounds.height *= 0.80f;

    for (const auto& obstacle : obstacles) {
        if (obstacle.passed) continue;

        sf::FloatRect obstacleBounds = obstacle.sprite.getGlobalBounds();

        // Check if in near-miss zone but not colliding
        if (playerBounds.intersects(obstacleBounds) && !tightPlayerBounds.intersects(obstacleBounds)) {
            return true;
        }
    }

    return false;
}

void Game::run() {
    sf::Clock clock;
    float accumulator = 0.0f;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.25f) {
            dt = 0.25f;
        }

        accumulator += dt;

        processEvents();
        inputManager.update();
        inputManager.updateMouse(window);

        while (accumulator >= Constants::TIME_STEP) {
            update(Constants::TIME_STEP);
            accumulator -= Constants::TIME_STEP;
        }

        render();
    }
}

void Game::processEvents() {
    sf::Event event;
    while (window.pollEvent(event)) {
        if (event.type == sf::Event::Closed) {
            window.close();
            continue;
        }

        if (event.type != sf::Event::KeyPressed) {
            continue;
        }

        // Menu system handles navigation for menu states
        if (currentState == GameState::Menu || currentState == GameState::Settings ||
            currentState == GameState::Shop || currentState == GameState::Achievements) {

            if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                menuSystem->navigateUp();
            }
            else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                menuSystem->navigateDown();
            }
            else if (event.key.code == sf::Keyboard::Left || event.key.code == sf::Keyboard::A) {
                menuSystem->navigateLeft();
            }
            else if (event.key.code == sf::Keyboard::Right || event.key.code == sf::Keyboard::D) {
                menuSystem->navigateRight();
            }
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                menuSystem->select(currentState, saveSystem);
            }
            else if (event.key.code == sf::Keyboard::Escape) {
                menuSystem->back(currentState);
                if (currentState == GameState::Menu) {
                    // If escape pressed in main menu, close window
                    // (back() returns to menu, so check if we're still at menu root)
                }
            }
            continue;
        }

        // Pause menu navigation
        if (currentState == GameState::Pause) {
            if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                menuSystem->navigateUp();
            }
            else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                menuSystem->navigateDown();
            }
            else if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                MenuSelection selection = menuSystem->getPauseSelection();
                if (selection == MenuSelection::Resume) {
                    currentState = GameState::Play;
                }
                else if (selection == MenuSelection::Settings) {
                    currentState = GameState::Settings;
                }
                else if (selection == MenuSelection::QuitToMenu) {
                    currentState = GameState::Menu;
                    menuSystem->resetMenuState();
                }
            }
            else if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P) {
                currentState = GameState::Play;
            }
            continue;
        }

        // In-game controls
        if (currentState == GameState::Play) {
            // When game over, only handle game over menu
            if (gameOver) {
                if (event.key.code == sf::Keyboard::Enter || event.key.code == sf::Keyboard::Space) {
                    MenuSelection selection = menuSystem->getGameOverSelection();
                    if (selection == MenuSelection::Retry) {
                        resetRun();
                    }
                    else if (selection == MenuSelection::QuitToMenu) {
                        currentState = GameState::Menu;
                        menuSystem->resetMenuState();
                    }
                }
                else if (event.key.code == sf::Keyboard::Up || event.key.code == sf::Keyboard::W) {
                    menuSystem->navigateUp();
                }
                else if (event.key.code == sf::Keyboard::Down || event.key.code == sf::Keyboard::S) {
                    menuSystem->navigateDown();
                }
                else if (event.key.code == sf::Keyboard::Escape) {
                    currentState = GameState::Menu;
                    menuSystem->resetMenuState();
                }
            }
            else {
                // Normal gameplay - only Escape pauses
                if (event.key.code == sf::Keyboard::Escape || event.key.code == sf::Keyboard::P) {
                    currentState = GameState::Pause;
                    menuSystem->resetPauseSelection();
                }
            }
        }
    }
}

void Game::update(float dt) {
    updateGameplayMusic();

    // Update screen effects regardless of state
    screenEffects->update(dt);

    // Check for state transitions FIRST (before processing current state)
    if (currentState == GameState::Play && previousState != GameState::Play && !gameOver) {
        // Transitioning TO Play state - start the game
        startGameFromMenu();
    }
    previousState = currentState;

    // Handle menu states
    if (currentState == GameState::Menu || currentState == GameState::Settings ||
        currentState == GameState::Shop || currentState == GameState::Achievements) {
        menuSystem->handleInput(inputManager, currentState);
        menuSystem->update(dt);
        particleSystem->update(dt);
        return;
    }

    if (currentState == GameState::Pause) {
        // Pause menu - navigation handled in processEvents()
        // Just update any visual effects here
        return;
    }

    if (!player || currentState != GameState::Play) {
        return;
    }

    if (gameOver) {
        // Game over - menu navigation handled in processEvents()
        // Just update particles for visual effect
        particleSystem->update(dt);
        return;
    }

    // Get time scale from power-ups
    float timeScale = powerUpManager->getSlowMoFactor();
    float scaledDt = dt * timeScale;

    // Unlimited ammo only when that power-up is active
    projectileManager->setUnlimitedAmmo(powerUpManager->isActive(PowerUpType::UnlimitedBullets));

    // Update run time and distance
    runTime += dt;
    distanceTraveled += obstacleBaseSpeed * dt * 0.1f;

    // Handle player input
    bool wasOnGround = playerWasOnGround;
    player->handleInput(inputManager);
    bool nowOnGround = player->isOnGround();

    // Jump sound: ground → air transition
    if (wasOnGround && !nowOnGround) {
        jumpSound.play();
    }
    playerWasOnGround = nowOnGround;

    // Handle shooting with mouse
    handleShooting();
    updateProjectiles(scaledDt);
    checkProjectileCollisions();

    // Update all entities
    for (auto* entity : entities) {
        entity->update(scaledDt);
    }

    // Step physics
    physics->step();

    // Clamp player to lane
    b2Body* playerBody = player->getBody();
    b2Vec2 velocity = playerBody->GetLinearVelocity();
    velocity.x = 0.0f;
    playerBody->SetLinearVelocity(velocity);

    b2Vec2 playerPos = playerBody->GetPosition();
    float laneXWorld = physics->toWorld(sf::Vector2f(laneX, 0.0f)).x;
    playerBody->SetTransform(b2Vec2(laneXWorld, playerPos.y), 0.0f);

    // Jump ceiling: player cannot rise above 28% from the top of the screen
    float winH = static_cast<float>(window.getSize().y);
    float ceilingScreenY = winH * 0.28f;
    float playerHalfPixels = Constants::PLAYER_HEIGHT * Constants::PPM * 0.5f;
    float ceilingCenterScreenY = ceilingScreenY + playerHalfPixels;
    b2Vec2 ceilingWorld = physics->toWorld(sf::Vector2f(0.0f, ceilingCenterScreenY));

    b2Vec2 curPos = playerBody->GetPosition();
    if (curPos.y > ceilingWorld.y) {
        // Cap upward velocity
        b2Vec2 curVel = playerBody->GetLinearVelocity();
        if (curVel.y > 0.0f) curVel.y = 0.0f;
        playerBody->SetLinearVelocity(curVel);
        playerBody->SetTransform(b2Vec2(curPos.x, ceilingWorld.y), 0.0f);
    }

    // Obstacle spawning - with maximum cap to prevent overcrowding
    constexpr size_t MAX_OBSTACLES_ON_SCREEN = 8;
    obstacleSpawnTimer += dt;
    if (obstacleSpawnTimer >= nextObstacleSpawnDelay && obstacles.size() < MAX_OBSTACLES_ON_SCREEN) {
        spawnObstacle();
        obstacleSpawnTimer = 0.0f;
        // Spawn delay scales down with speed but maintains a minimum gap
        // Base delay range widened by 20% to reduce obstacle density
        float spawnScale = std::max(0.60f, 1.0f - (obstacleBaseSpeed - 344.0f) / 700.0f);
        nextObstacleSpawnDelay = randomRange(rng, 1.02f, 1.92f) * spawnScale;
    }

    // Update obstacles
    updateObstacles(scaledDt);
    updateProfessors(dt);
    updateAssignments(dt);

    // Update power-ups and coins
    powerUpManager->update(dt, obstacleBaseSpeed, rng);
    coinManager->update(dt, obstacleBaseSpeed, rng);

    // Guaranteed ammo drop cadence by distance traveled (every 100-200m).
    while (distanceTraveled >= nextAmmoDropDistance) {
        float spawnX = static_cast<float>(window.getSize().x) + 100.0f;
        float spawnY = groundY - 80.0f;
        powerUpManager->spawnPowerUp(PowerUpType::Ammo, spawnX, spawnY, obstacleBaseSpeed);
        nextAmmoDropDistance += randomRange(
            rng,
            Constants::PROJECTILE_AMMO_DROP_DISTANCE_MIN,
            Constants::PROJECTILE_AMMO_DROP_DISTANCE_MAX
        );
    }

    // Check power-up collection
    if (player) {
        sf::FloatRect playerBounds = player->getBoundsScreen();
        PowerUpType collectedType;
        if (powerUpManager->checkCollection(playerBounds, &collectedType)) {
            // Update HUD with active power-ups
            if (collectedType == PowerUpType::Ammo) {
                projectileManager->addAmmo(Constants::PROJECTILE_AMMO_PICKUP);
                hud->triggerScorePopup("+" + std::to_string(Constants::PROJECTILE_AMMO_PICKUP) + " ammo",
                    player->getScreenPosition());
                hud->showToast("Ammo picked up", Constants::UI_SECONDARY);
                powerupSound.play();
            } else {
                powerupSound.play();
                for (const auto& effect : powerUpManager->getActiveEffects()) {
                    std::string name;
                    sf::Color color;
                    switch (effect.type) {
                        case PowerUpType::Shield:
                            name = "Shield"; color = sf::Color(100, 180, 255); break;
                        case PowerUpType::SlowMotion:
                            name = "SlowMo"; color = sf::Color(180, 100, 255); break;
                        case PowerUpType::DoubleScore:
                            name = "Double"; color = sf::Color(255, 215, 0); break;
                        case PowerUpType::Magnet:
                            name = "Magnet"; color = sf::Color(255, 100, 150); break;
                        case PowerUpType::UnlimitedBullets:
                            name = "Unlimited"; color = sf::Color(80, 220, 255); break;
                        default: name = "Power"; color = sf::Color::White;
                    }
                    hud->addPowerUp(name, color, effect.maxTime);
                }
                hud->showToast("Power-Up!", Constants::UI_ACCENT);
            }
        }

        // Check coin collection
        bool hasMagnet = powerUpManager->hasMagnet();
        int coinsCollected = coinManager->checkCollection(playerBounds, hasMagnet, player->getScreenPosition());
        if (coinsCollected > 0) {
            // Double coins upgrade: 2x coin value
            if (saveSystem && saveSystem->getData().doubleCoinsUpgrade) coinsCollected *= 2;
            coinsThisRun += coinsCollected;
            if (saveSystem) {
                saveSystem->addCoins(coinsCollected);
            }
            hud->triggerScorePopup("+" + std::to_string(coinsCollected) + " coin", player->getScreenPosition());
            coinSound.play();
        }
    }

    // Check for near-miss bonus
    if (checkNearMiss()) {
        comboSystem->registerNearMiss();
        score += Constants::NEAR_MISS_BONUS * powerUpManager->getScoreMultiplier();
        hud->triggerScorePopup("NEAR MISS!", sf::Vector2f(laneX + 80.0f, groundY - 150.0f));
        hud->showToast("Close call!", Constants::UI_SECONDARY);
    }

    // Update score
    float scoreMultiplier = powerUpManager->getScoreMultiplier();
    score += dt * 14.0f * scoreMultiplier;

    // Update combo system
    comboSystem->update(dt);

    // Check combo milestones
    if (comboSystem->hasNewMilestone()) {
        hud->showMilestoneToast(std::to_string(comboSystem->getMilestoneValue()) + "x COMBO!");
        hud->triggerComboFlash();
        comboSystem->clearMilestoneFlag();
    }

    // Update difficulty with smoother progression (slower early ramp, faster later ramp)
    constexpr float BASE_OBSTACLE_SPEED = 344.0f;   // 20% slower initial speed
    constexpr float TIME_TO_MAX_DIFFICULTY = 150.0f;  // Seconds to reach top speed
    float progress = std::min(runTime / TIME_TO_MAX_DIFFICULTY, 1.0f);
    float easedProgress = progress * progress;
    float targetSpeed = BASE_OBSTACLE_SPEED +
        (Constants::MAX_OBSTACLE_SPEED - BASE_OBSTACLE_SPEED) * easedProgress;

    // Keep per-frame speed changes smooth even when the target shifts.
    obstacleBaseSpeed = std::min(targetSpeed, obstacleBaseSpeed + dt * Constants::DIFFICULTY_RAMP_RATE);

    // Update visual systems
    particleSystem->update(dt);

    // Update HUD
    hud->setScore(score);
    hud->setBestScore(bestScore);
    hud->setCoins(coinsThisRun);
    hud->setCombo(comboSystem->getComboCount(), comboSystem->getMultiplier());
    hud->setSpeed(obstacleBaseSpeed, Constants::MAX_OBSTACLE_SPEED);
    hud->setDistance(distanceTraveled);
    hud->setAmmo(projectileManager->getAmmo(), projectileManager->getMaxAmmo(), projectileManager->hasUnlimitedAmmo());
    hud->update(dt);

    // Update power-up display in HUD
    for (const auto& effect : powerUpManager->getActiveEffects()) {
        std::string name;
        switch (effect.type) {
            case PowerUpType::Shield: name = "Shield"; break;
            case PowerUpType::SlowMotion: name = "SlowMo"; break;
            case PowerUpType::DoubleScore: name = "Double"; break;
            case PowerUpType::Magnet: name = "Magnet"; break;
            case PowerUpType::UnlimitedBullets: name = "Unlimited"; break;
            default: name = "Power";
        }
        hud->updatePowerUp(name, effect.remainingTime);
    }

    // Parkour landing detection
    checkParkourLanding();

    // Assignment vs bullet and player
    checkAssignmentBulletCollision();
    checkAssignmentPlayerCollision();

    // Check collision
    if (checkObstacleCollision()) {
        if (powerUpManager->hasShield()) {
            // Shield absorbs hit
            powerUpManager->consumeShield();
            hud->removePowerUp("Shield");
            screenEffects->triggerShake(8.0f);
            screenEffects->triggerFlash(sf::Color(100, 180, 255, 100), 0.15f);
            hud->showToast("Shield broken!", sf::Color(100, 180, 255));
            player->setInvulnerable(0.5f);
        } else if (extraLivesRemaining > 0) {
            // Extra life absorbs hit
            extraLivesRemaining--;
            player->setInvulnerable(2.0f);
            screenEffects->triggerShake(10.0f);
            screenEffects->triggerFlash(sf::Color(255, 80, 80, 160), 0.25f);
            hud->showToast("Extra Life used! (" + std::to_string(extraLivesRemaining) + " left)",
                           sf::Color(255, 120, 50));
        } else {
            handleGameOver();
        }
    }

    // Check achievements
    checkAchievements();

    previousState = currentState;
}

void Game::updateGameplayMusic() {
    const bool inPlay = (currentState == GameState::Play && !gameOver);

    if (inPlay) {
        if (gameplayMusic.getStatus() != sf::SoundSource::Playing)
            gameplayMusic.play();
        if (menuMusic.getStatus() == sf::SoundSource::Playing)
            menuMusic.pause();
    } else {
        if (gameplayMusic.getStatus() == sf::SoundSource::Playing)
            gameplayMusic.pause();
        if (menuMusic.getStatus() != sf::SoundSource::Playing)
            menuMusic.play();
    }
}

void Game::handleGameOver() {
    gameOver = true;

    // Update best score
    if (score > bestScore) {
        bestScore = score;
        hud->showAchievementToast("New Best Score!");
    }

    // Save progress
    if (saveSystem) {
        saveSystem->updateBestScore(score);
        saveSystem->addPlayTime(runTime);
        saveSystem->addObstaclesDodged(obstaclesDodged);
        saveSystem->updateHighestCombo(comboSystem->getComboCount());
        saveSystem->save();
    }

    // Audio
    gameplayMusic.pause();
    gameLostSound.play();

    // Trigger effects
    screenEffects->triggerShake(Constants::SCREEN_SHAKE_INTENSITY);
    screenEffects->triggerFlash(sf::Color(255, 50, 50, 150), 0.2f);
    hud->triggerDamageFlash();
    comboSystem->breakCombo();

    // Check end-of-run achievements
    if (achievementSystem) {
        achievementSystem->onRunComplete(score, coinsThisRun, obstaclesDodged, comboSystem->getComboCount());
    }
}

void Game::checkParkourLanding() {
    if (!player) return;

    sf::FloatRect pBounds = player->getBoundsScreen();
    float playerBottom = pBounds.top + pBounds.height;

    for (auto& obstacle : obstacles) {
        if (!obstacle.physBody) continue;   // Only bench/chair are platforms
        if (obstacle.passed) continue;

        sf::FloatRect oBounds = obstacle.sprite.getGlobalBounds();

        // Horizontal overlap check — is the player above this obstacle?
        bool hOverlap = (pBounds.left + pBounds.width > oBounds.left) &&
                        (pBounds.left < oBounds.left + oBounds.width);
        if (!hOverlap) {
            // No longer over it — clear flag
            obstacle.playerOnTop = false;
            continue;
        }

        // Player is standing with feet at obstacle top surface (within 18px tolerance)
        bool onTopNow = player->isOnGround() &&
                        (playerBottom >= oBounds.top - 5.0f) &&
                        (playerBottom < oBounds.top + 18.0f);

        if (onTopNow && !obstacle.playerOnTop) {
            // Just landed on the obstacle — trigger parkour effects
            obstacle.playerOnTop = true;
            landSound.play();

            // Small upward vault bounce so the player pops off the surface
            b2Body* pb = player->getBody();
            b2Vec2 vel = pb->GetLinearVelocity();
            vel.y += 5.5f;
            pb->SetLinearVelocity(vel);

            // Visual / score feedback
            screenEffects->triggerShake(4.0f);
            screenEffects->triggerFlash(sf::Color(255, 220, 80, 60), 0.1f);
            score += 30.0f * powerUpManager->getScoreMultiplier();
            hud->triggerScorePopup("PARKOUR! +30",
                sf::Vector2f(laneX + 60.0f, oBounds.top - 30.0f));
            comboSystem->registerDodge();
        }
        else if (!onTopNow) {
            obstacle.playerOnTop = false;
        }
    }
}

void Game::updateProfessors(float dt) {
    if (!player) return;

    sf::Texture& profTex = assets->get<sf::Texture>(AssetKeys::OBSTACLE_PROFESSOR);
    sf::Vector2u ts = profTex.getSize();
    int fw = static_cast<int>(ts.x) / RunnerObstacle::PROF_COLS;
    int fh = static_cast<int>(ts.y) / RunnerObstacle::PROF_ROWS;

    float timeScale = powerUpManager->getSlowMoFactor();

    for (auto& obs : obstacles) {
        if (obs.type != ObstacleType::Professor) continue;

        // --- Dying (disappear) animation ---
        if (obs.dying) {
            obs.dyingTimer += dt;
            float t = obs.dyingTimer / RunnerObstacle::DYING_DURATION;  // 0→1
            sf::Uint8 alpha = static_cast<sf::Uint8>(255 * (1.0f - t));
            obs.sprite.setColor(sf::Color(255, 200, 100, alpha));
            // Spin during disappear
            obs.sprite.setRotation(obs.sprite.getRotation() + 400.0f * dt);
            // Scale up slightly as they vanish
            float s = 0.55f * (1.0f + t * 0.4f);
            obs.sprite.setScale(s, s);
            continue;  // Don't animate frames or throw while dying
        }

        // --- Spritesheet frame animation ---
        obs.profFrameTimer += dt * timeScale;
        if (obs.profFrameTimer >= RunnerObstacle::PROF_FRAME_TIME) {
            obs.profFrameTimer -= RunnerObstacle::PROF_FRAME_TIME;
            obs.profFrame = (obs.profFrame + 1) % RunnerObstacle::PROF_FRAME_COUNT;
        }
        int col = obs.profFrame % RunnerObstacle::PROF_COLS;
        int row = obs.profFrame / RunnerObstacle::PROF_COLS;
        obs.sprite.setTextureRect(sf::IntRect(col * fw, row * fh, fw, fh));

        // --- Throw assignment projectile (max 3 total) ---
        if (obs.assignmentsThrown < RunnerObstacle::PROF_MAX_THROWS && !obs.passed) {
            obs.profThrowTimer -= dt * timeScale;
            if (obs.profThrowTimer <= 0.0f) {
                obs.profThrowTimer = RunnerObstacle::PROF_THROW_DELAY;
                obs.assignmentsThrown++;

                sf::FloatRect pb = obs.sprite.getGlobalBounds();
                Assignment asgn;
                asgn.position      = sf::Vector2f(pb.left, pb.top + pb.height * 0.35f);
                sf::Vector2f playerPos = player->getScreenPosition();
                sf::Vector2f dir   = playerPos - asgn.position;
                float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
                if (len > 1.0f) dir /= len;
                asgn.velocity      = dir * obs.speed * 1.6f;
                asgn.rotation      = 0.0f;
                asgn.rotationSpeed = 280.0f;
                asgn.hitPoints     = 2;
                asgn.active        = true;
                asgn.animTimer     = 0.0f;
                asgn.currentFrame  = 0;
                asgn.ownerProfId   = obs.profId;
                assignments.push_back(asgn);
            }
        }

        // Dying is triggered immediately in checkAssignmentBulletCollision
    }

    // Erase fully-faded dying professors
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(), [](const RunnerObstacle& o) {
            return o.type == ObstacleType::Professor && o.dying &&
                   o.dyingTimer >= RunnerObstacle::DYING_DURATION;
        }),
        obstacles.end()
    );
}

void Game::updateAssignments(float dt) {
    float timeScale = powerUpManager->getSlowMoFactor();

    for (auto& a : assignments) {
        if (!a.active) continue;
        a.position += a.velocity * dt * timeScale;
        a.rotation += a.rotationSpeed * dt * timeScale;

        // Animate frames (use book texture as spinning paper)
        a.animTimer += dt;
        if (a.animTimer >= Assignment::FRAME_TIME) {
            a.animTimer -= Assignment::FRAME_TIME;
            a.currentFrame = (a.currentFrame + 1) % Assignment::MAX_FRAME;
        }

        // Deactivate when off the left edge
        if (a.position.x < -80.0f) a.active = false;
    }

    assignments.remove_if([](const Assignment& a) { return !a.active; });
}

void Game::checkAssignmentPlayerCollision() {
    if (!player || player->isInvulnerable()) return;

    sf::FloatRect pBounds = player->getBoundsScreen();
    pBounds.left   += pBounds.width  * 0.22f;
    pBounds.top    += pBounds.height * 0.12f;
    pBounds.width  *= 0.56f;
    pBounds.height *= 0.78f;

    for (auto& a : assignments) {
        if (!a.active) continue;
        sf::FloatRect aBounds(a.position.x - 20.0f, a.position.y - 20.0f, 40.0f, 40.0f);
        if (pBounds.intersects(aBounds)) {
            a.active = false;
            if (powerUpManager->hasShield()) {
                powerUpManager->consumeShield();
                hud->removePowerUp("Shield");
                screenEffects->triggerShake(6.0f);
                screenEffects->triggerFlash(sf::Color(100, 180, 255, 100), 0.12f);
                hud->showToast("Shield blocked assignment!", sf::Color(100, 180, 255));
                player->setInvulnerable(0.5f);
            } else {
                handleGameOver();
            }
        }
    }
}

void Game::checkAssignmentBulletCollision() {
    auto& projectiles = projectileManager->getProjectilesMutable();

    for (auto& proj : projectiles) {
        if (!proj.active) continue;
        for (auto& a : assignments) {
            if (!a.active) continue;
            // Generous hit radius — bullet does not need to be pixel-perfect
            constexpr float HIT_RADIUS = 70.0f;
            sf::Vector2f diff = proj.position - a.position;
            float distSq = diff.x * diff.x + diff.y * diff.y;
            if (distSq > HIT_RADIUS * HIT_RADIUS) continue;

            proj.active = false;
            a.hitPoints--;

            screenEffects->triggerShake(2.5f);
            particleSystem->spawnPaperDestruction(a.position);

            if (a.hitPoints <= 0) {
                a.active = false;
                score += 25.0f * powerUpManager->getScoreMultiplier();
                hud->triggerScorePopup("BLOCKED! +25", a.position);

                // Large paper-burst at assignment position
                particleSystem->spawnPaperDestruction(a.position);
                screenEffects->triggerFlash(sf::Color(255, 240, 100, 120), 0.12f);

                // Find owning professor by ID and trigger disappear immediately
                RunnerObstacle* prof = nullptr;
                for (auto& obs : obstacles) {
                    if (obs.type == ObstacleType::Professor && obs.profId == a.ownerProfId) {
                        prof = &obs;
                        break;
                    }
                }
                if (prof && !prof->dying) {
                    prof->dying      = true;
                    // Set timer past DYING_DURATION so updateProfessors erases it next frame
                    prof->dyingTimer = RunnerObstacle::DYING_DURATION + 1.0f;

                    // Hide the professor sprite immediately
                    prof->sprite.setColor(sf::Color(0, 0, 0, 0));

                    // Burst effects at professor position
                    sf::FloatRect gb = prof->sprite.getGlobalBounds();
                    sf::Vector2f profCentre(gb.left + gb.width * 0.5f,
                                           gb.top  + gb.height * 0.5f);
                    particleSystem->spawnExplosion(profCentre, sf::Color(255, 220, 60), 40);
                    particleSystem->spawnPaperDestruction(profCentre);
                    particleSystem->spawnExplosion(profCentre, sf::Color(255, 255, 200), 20);

                    screenEffects->triggerShake(8.0f);
                    screenEffects->triggerFlash(sf::Color(255, 200, 50, 180), 0.20f);

                    int bonus = static_cast<int>(100.0f * powerUpManager->getScoreMultiplier());
                    score += bonus;
                    hud->triggerScorePopup("Professor fled! +" + std::to_string(bonus), profCentre);
                    hud->showToast("Professor expelled!", sf::Color(255, 200, 100));
                    comboSystem->registerDodge();
                }
            } else {
                hud->triggerScorePopup("HIT!", a.position);
            }
            break;
        }
    }
}

void Game::checkAchievements() {
    if (!achievementSystem || !saveSystem) return;

    // Check current state achievements
    achievementSystem->checkAchievements(
        score,
        comboSystem->getComboCount(),
        saveSystem->getData().totalCoins,
        saveSystem->getData().totalRuns
    );

    // Process newly unlocked achievements
    auto newAchievements = achievementSystem->popNewlyUnlocked();
    for (const auto& ach : newAchievements) {
        hud->showAchievementToast(ach.name);
    }
}

void Game::render() {
    // Apply screen effects
    sf::View effectView = window.getDefaultView();
    screenEffects->applyToView(effectView);

    window.clear(sf::Color(8, 10, 15));
    window.setView(effectView);

    if (parallaxBg) {
        parallaxBg->render(window, effectView);
    }

    // Render menu states
    if (currentState == GameState::Menu || currentState == GameState::Settings ||
        currentState == GameState::Shop || currentState == GameState::Achievements) {
        sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
        menuSystem->render(window, font, currentState, saveSystem);
        screenEffects->renderOverlays(window);
        window.display();
        return;
    }

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Ground band
    sf::RectangleShape groundBand;
    groundBand.setSize(sf::Vector2f(winW, winH - groundY));
    groundBand.setPosition(0.0f, groundY);
    groundBand.setFillColor(sf::Color(18, 22, 32, 220));
    window.draw(groundBand);

    // Track line
    sf::VertexArray trackLine(sf::Lines, 2);
    trackLine[0].position = sf::Vector2f(0.0f, groundY);
    trackLine[0].color = sf::Color(130, 145, 168);
    trackLine[1].position = sf::Vector2f(winW, groundY);
    trackLine[1].color = sf::Color(130, 145, 168);
    window.draw(trackLine);

    // Jump ceiling guide line removed (invisible ceiling)

    // Draw player shadow
    if (player) {
        sf::FloatRect pBounds = player->getBoundsScreen();
        sf::CircleShape playerShadow(pBounds.width * 0.4f);
        playerShadow.setScale(1.0f, 0.25f);
        playerShadow.setFillColor(sf::Color(0, 0, 0, 50));
        playerShadow.setOrigin(playerShadow.getRadius(), playerShadow.getRadius());
        playerShadow.setPosition(pBounds.left + pBounds.width * 0.5f, groundY + 4.0f);
        window.draw(playerShadow);
    }

    // Draw entities (player)
    for (auto* entity : entities) {
        entity->render(window);
    }

    // Draw coins
    coinManager->render(window);

    // Draw power-ups
    powerUpManager->render(window);

    // Draw obstacles with shadows
    for (const auto& obstacle : obstacles) {
        // Draw shadow under obstacle
        sf::FloatRect bounds = obstacle.sprite.getGlobalBounds();
        sf::CircleShape shadow(bounds.width * 0.35f);
        shadow.setScale(1.0f, 0.3f);
        shadow.setFillColor(sf::Color(0, 0, 0, 45));
        shadow.setOrigin(shadow.getRadius(), shadow.getRadius());
        shadow.setPosition(obstacle.sprite.getPosition().x, groundY + 4.0f);
        window.draw(shadow);

        window.draw(obstacle.sprite);
        
        // Reset color if it was flashed from being hit
        const_cast<RunnerObstacle&>(obstacle).sprite.setColor(sf::Color::White);
    }

    // Draw assignment projectiles (thrown by professor)
    {
        sf::Texture& asgnTex = assets->get<sf::Texture>(AssetKeys::ASSIGNMENT);
        sf::Sprite asgnSprite(asgnTex);
        sf::Vector2u ts = asgnTex.getSize();
        asgnSprite.setOrigin(ts.x * 0.5f, ts.y * 0.5f);
        asgnSprite.setScale(0.22f, 0.22f);   // Large, readable projectile
        for (const auto& a : assignments) {
            if (!a.active) continue;
            asgnSprite.setColor(sf::Color::White);
            asgnSprite.setPosition(a.position);
            asgnSprite.setRotation(a.rotation);
            window.draw(asgnSprite);
        }
    }

    // Draw projectiles (bullets)
    projectileManager->render(window);

    // Particles (now includes all destruction effects)
    if (particleSystem) {
        particleSystem->render(window);
    }

    // Lighting
    if (lightingSystem && player) {
        lightingSystem->clearLights();
        b2Vec2 bodyPos = player->getBody()->GetPosition();
        sf::Vector2f screenPos = physics->toScreen(bodyPos);

        // Main player light
        lightingSystem->addLight(
            screenPos,
            260.0f,
            sf::Color(254, 225, 190),
            0.55f,
            0.18f
        );

        // Add glowing lights for power-ups
        for (const auto& effect : powerUpManager->getActiveEffects()) {
            sf::Color color;
            switch (effect.type) {
                case PowerUpType::Shield: color = sf::Color(100, 180, 255); break;
                case PowerUpType::SlowMotion: color = sf::Color(180, 100, 255); break;
                case PowerUpType::DoubleScore: color = sf::Color(255, 215, 0); break;
                case PowerUpType::Magnet: color = sf::Color(255, 100, 150); break;
                default: color = sf::Color::White;
            }
            lightingSystem->addLight(screenPos, 180.0f, color, 0.3f, 0.1f);
        }

        lightingSystem->render(window, Constants::TIME_STEP);
    }

    // Vignette
    renderVignette();

    // Screen effects overlays
    screenEffects->renderOverlays(window);

    // Draw HUD (always on top)
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    if (!gameOver && currentState == GameState::Play) {
        hud->render(window, font);
    }

    // Pause overlay
    if (currentState == GameState::Pause) {
        menuSystem->render(window, font, currentState, saveSystem);
    }

    // Game over overlay
    if (gameOver) {
        // Pass stats to menu system for game over screen
        menuSystem->setGameOverStats(score, bestScore, coinsThisRun, obstaclesDodged,
                                      comboSystem->getComboCount(), distanceTraveled);
        menuSystem->render(window, font, GameState::GameOver, saveSystem);
    }

    window.display();
}

void Game::renderMenu() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);

    sf::RectangleShape band;
    band.setSize(sf::Vector2f(winW, 190.0f));
    band.setPosition(0.0f, winH * 0.30f);
    band.setFillColor(sf::Color(8, 11, 16, 200));
    window.draw(band);

    sf::Text title;
    title.setFont(font);
    title.setString("DEADLINE'S EDGE  CAMPUS RUN");
    title.setCharacterSize(52);
    title.setFillColor(sf::Color(230, 235, 245));
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.left + titleBounds.width * 0.5f,
                    titleBounds.top + titleBounds.height * 0.5f);
    title.setPosition(winW * 0.5f, winH * 0.38f);
    window.draw(title);

    sf::Text sub;
    sub.setFont(font);
    sub.setString("Dino-style endless runner with student chaos");
    sub.setCharacterSize(20);
    sub.setFillColor(sf::Color(156, 168, 190));
    sf::FloatRect subBounds = sub.getLocalBounds();
    sub.setOrigin(subBounds.left + subBounds.width * 0.5f,
                  subBounds.top + subBounds.height * 0.5f);
    sub.setPosition(winW * 0.5f, winH * 0.46f);
    window.draw(sub);

    float pulse = (std::sin(static_cast<float>(std::clock()) / CLOCKS_PER_SEC * 4.0f) + 1.0f) * 0.5f;
    sf::Text prompt;
    prompt.setFont(font);
    prompt.setString("Press ENTER to start sprinting");
    prompt.setCharacterSize(24);
    prompt.setFillColor(sf::Color(200, 210, 230, static_cast<sf::Uint8>(120 + pulse * 120)));
    sf::FloatRect promptBounds = prompt.getLocalBounds();
    prompt.setOrigin(promptBounds.left + promptBounds.width * 0.5f,
                     promptBounds.top + promptBounds.height * 0.5f);
    prompt.setPosition(winW * 0.5f, winH * 0.64f);
    window.draw(prompt);

    sf::Text controls;
    controls.setFont(font);
    controls.setString("Jump over chairs and benches. Duck under flying textbooks!");
    controls.setCharacterSize(14);
    controls.setFillColor(sf::Color(130, 142, 164, 190));
    sf::FloatRect ctrlBounds = controls.getLocalBounds();
    controls.setOrigin(ctrlBounds.left + ctrlBounds.width * 0.5f,
                       ctrlBounds.top + ctrlBounds.height * 0.5f);
    controls.setPosition(winW * 0.5f, winH * 0.71f);
    window.draw(controls);

    if (particleSystem) {
        particleSystem->render(window);
    }
}

void Game::renderPause() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);

    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(0, 0, 0, 165));
    window.draw(overlay);

    sf::Text text;
    text.setFont(font);
    text.setString("PAUSED");
    text.setCharacterSize(42);
    text.setFillColor(sf::Color(230, 236, 248));
    sf::FloatRect b = text.getLocalBounds();
    text.setOrigin(b.left + b.width * 0.5f, b.top + b.height * 0.5f);
    text.setPosition(winW * 0.5f, winH * 0.45f);
    window.draw(text);

    sf::Text hint;
    hint.setFont(font);
    hint.setString("P or ESC to continue | Q for menu");
    hint.setCharacterSize(18);
    hint.setFillColor(sf::Color(170, 180, 200));
    sf::FloatRect hb = hint.getLocalBounds();
    hint.setOrigin(hb.left + hb.width * 0.5f, hb.top + hb.height * 0.5f);
    hint.setPosition(winW * 0.5f, winH * 0.53f);
    window.draw(hint);
}

void Game::renderGameOver() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);

    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(10, 0, 0, 170));
    window.draw(overlay);

    sf::Text gameOverText;
    gameOverText.setFont(font);
    gameOverText.setString("ASSIGNMENT COLLISION");
    gameOverText.setCharacterSize(50);
    gameOverText.setFillColor(sf::Color(244, 215, 215));
    sf::FloatRect gb = gameOverText.getLocalBounds();
    gameOverText.setOrigin(gb.left + gb.width * 0.5f, gb.top + gb.height * 0.5f);
    gameOverText.setPosition(winW * 0.5f, winH * 0.42f);
    window.draw(gameOverText);

    sf::Text restartText;
    restartText.setFont(font);
    restartText.setString("Press ENTER to retry");
    restartText.setCharacterSize(24);
    restartText.setFillColor(sf::Color(235, 230, 240));
    sf::FloatRect rb = restartText.getLocalBounds();
    restartText.setOrigin(rb.left + rb.width * 0.5f, rb.top + rb.height * 0.5f);
    restartText.setPosition(winW * 0.5f, winH * 0.53f);
    window.draw(restartText);
}

void Game::renderVignette() {
    window.draw(vignetteTop);
    window.draw(vignetteBottom);
    window.draw(vignetteLeft);
    window.draw(vignetteRight);

    if (!gameOver) {
        return;
    }

    sf::RectangleShape crashTint;
    crashTint.setSize(sf::Vector2f(
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)
    ));
    crashTint.setFillColor(sf::Color(80, 10, 10, 65));
    window.draw(crashTint);
}

void Game::handleShooting() {
    if (!player || gameOver) return;

    // Fire on left mouse click
    if (inputManager.isMouseButtonPressed(sf::Mouse::Left)) {
        sf::Vector2f playerPos = player->getScreenPosition();
        sf::Vector2f mousePos = inputManager.getMousePosition();
        
        // Fire bullet from player center toward mouse
        if (projectileManager->fire(playerPos, mousePos)) {
            bulletSound.play();
        }
    }
}

void Game::updateProjectiles(float dt) {
    projectileManager->update(dt);
}

void Game::checkProjectileCollisions() {
    auto& projectiles = projectileManager->getProjectilesMutable();

    for (auto& proj : projectiles) {
        if (!proj.active) continue;

        // Check collision with each obstacle
        for (auto& obstacle : obstacles) {
            if (obstacle.hitPoints <= 0) continue;
            // Professor cannot be shot directly — destroy its assignments instead
            if (obstacle.type == ObstacleType::Professor) continue;

            sf::FloatRect obstacleBounds = obstacle.sprite.getGlobalBounds();
            
            // Simple point-in-rect collision for bullet
            if (obstacleBounds.contains(proj.position)) {
                // Hit the obstacle
                obstacle.hitPoints--;
                proj.active = false;  // Destroy bullet

                // Visual feedback: flash the obstacle
                obstacle.sprite.setColor(sf::Color(255, 150, 150));

                // Trigger screen effects
                screenEffects->triggerShake(3.0f);

                // Get obstacle center for particle effects
                sf::Vector2f obstacleCenter(
                    obstacleBounds.left + obstacleBounds.width * 0.5f,
                    obstacleBounds.top + obstacleBounds.height * 0.5f
                );

                // Determine color based on obstacle type
                sf::Color explosionColor;
                switch (obstacle.type) {
                    case ObstacleType::Chair:
                        explosionColor = sf::Color(139, 90, 43);   // Brown wood
                        break;
                    case ObstacleType::Bench:
                        explosionColor = sf::Color(101, 67, 33);   // Dark wood
                        break;
                    case ObstacleType::Book:
                        explosionColor = sf::Color(200, 180, 140); // Paper/beige
                        break;
                    case ObstacleType::Professor:
                        explosionColor = sf::Color(80, 80, 100);   // Dark cloth
                        break;
                    case ObstacleType::ExamStack:
                        explosionColor = sf::Color(240, 240, 230); // White paper
                        break;
                    default:
                        explosionColor = sf::Color(150, 130, 100);
                }

                if (obstacle.hitPoints <= 0) {
                    // Obstacle destroyed
                    if (obstacle.type == ObstacleType::Professor) {
                        // Professor: start disappear animation instead of instant removal
                        obstacle.dying = true;
                        obstacle.dyingTimer = 0.0f;
                        // Reset rotation origin to sprite centre
                        sf::FloatRect lb = obstacle.sprite.getLocalBounds();
                        obstacle.sprite.setOrigin(lb.width * 0.5f, lb.height * 0.5f);
                        screenEffects->triggerShake(6.0f);
                        screenEffects->triggerFlash(sf::Color(255, 200, 80, 100), 0.15f);
                        particleSystem->spawnExplosion(obstacleCenter, sf::Color(255, 220, 60), 25);
                    } else {
                        switch (obstacle.type) {
                            case ObstacleType::Chair:
                            case ObstacleType::Bench:
                                particleSystem->spawnWoodDestruction(obstacleCenter);
                                break;
                            case ObstacleType::Book:
                            case ObstacleType::ExamStack:
                                particleSystem->spawnPaperDestruction(obstacleCenter);
                                break;
                            default:
                                particleSystem->spawnExplosion(obstacleCenter, explosionColor, 30);
                                break;
                        }
                    }

                    // Give bonus points
                    int destroyBonus = (obstacle.type == ObstacleType::Professor) ? 100 : 50;
                    float scoreMultiplier = powerUpManager->getScoreMultiplier();
                    int finalBonus = static_cast<int>(destroyBonus * scoreMultiplier);
                    score += finalBonus;

                    hud->triggerScorePopup("DESTROYED! +" + std::to_string(finalBonus),
                        sf::Vector2f(obstacle.sprite.getPosition().x,
                                     obstacle.sprite.getPosition().y - 50.0f));
                    hud->showToast(obstacle.type == ObstacleType::Professor
                        ? "Professor expelled!" : "Object destroyed!", sf::Color(255, 200, 100));

                    screenEffects->triggerShake(8.0f);
                    screenEffects->triggerFlash(sf::Color(255, 200, 100, 80), 0.1f);
                } else {
                    // Still has HP - spawn small hit effect
                    particleSystem->spawnHitEffect(proj.position, explosionColor, 10);
                    
                    // Show hit popup
                    hud->triggerScorePopup("HIT!",
                        sf::Vector2f(obstacle.sprite.getPosition().x,
                                     obstacle.sprite.getPosition().y - 30.0f));
                }

                break;  // One bullet can only hit one obstacle
            }
        }
    }

    // Remove non-professor destroyed obstacles immediately.
    // Professors stay in the list while their dying animation plays (updateProfessors erases them).
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(), [](const RunnerObstacle& o) {
            return o.hitPoints <= 0 && o.type != ObstacleType::Professor;
        }),
        obstacles.end()
    );
}
