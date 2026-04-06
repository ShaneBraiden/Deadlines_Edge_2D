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
    , obstacleBaseSpeed(430.0f)
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

    // Set particle system textures
    particleSystem->setTextures(
        &assets->get<sf::Texture>(AssetKeys::EXPLOSION),
        &assets->get<sf::Texture>(AssetKeys::HIT_SPARK),
        &assets->get<sf::Texture>(AssetKeys::SMOKE_PUFF)
    );

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
    
    // Set projectile textures
    projectileManager->setTextures(
        &assets->get<sf::Texture>(AssetKeys::BULLET),
        &assets->get<sf::Texture>(AssetKeys::MUZZLE_FLASH)
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
    nextObstacleSpawnDelay = 1.1f;
    obstacleBaseSpeed = 430.0f;
    score = 0.0f;
    gameOver = false;

    // Reset run statistics
    runTime = 0.0f;
    obstaclesDodged = 0;
    coinsThisRun = 0;
    distanceTraveled = 0.0f;

    // Reset systems
    comboSystem->reset();
    powerUpManager->reset();
    coinManager->reset();
    screenEffects->reset();
    hud->clearPowerUps();
    projectileManager->reset();

    // Increment run count
    if (saveSystem) {
        saveSystem->incrementRuns();
    }
}

void Game::spawnObstacle() {
    std::uniform_int_distribution<int> typeDist(0, 99);
    int roll = typeDist(rng);

    RunnerObstacle obstacle;
    obstacle.speed = obstacleBaseSpeed + randomRange(rng, -25.0f, 90.0f);
    obstacle.passed = false;
    obstacle.animTimer = 0.0f;
    obstacle.hitPoints = RunnerObstacle::DEFAULT_HIT_POINTS;  // 2 hits to destroy

    float spawnX = static_cast<float>(window.getSize().x) + randomRange(rng, 60.0f, 260.0f);
    float spawnY = groundY;

    // Prevent obstacle overlap: ensure minimum spacing from existing obstacles
    constexpr float MIN_OBSTACLE_SPACING = 180.0f;  // Minimum pixels between obstacles
    for (const auto& existingObstacle : obstacles) {
        float existingX = existingObstacle.sprite.getPosition().x;
        float existingWidth = existingObstacle.sprite.getGlobalBounds().width;
        // Check if proposed spawn position is too close to an existing obstacle
        if (std::abs(spawnX - existingX) < MIN_OBSTACLE_SPACING + existingWidth * 0.5f) {
            // Push spawn position further right to avoid overlap
            spawnX = existingX + MIN_OBSTACLE_SPACING + existingWidth * 0.5f + randomRange(rng, 20.0f, 80.0f);
        }
    }

    // Lab: arrow operator + template method call
    if (roll < 35) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_CHAIR));
        obstacle.sprite.setScale(4.5f, 4.5f);
        obstacle.type = ObstacleType::Chair;
    }
    else if (roll < 60) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_BENCH));
        obstacle.sprite.setScale(3.2f, 3.2f);
        obstacle.type = ObstacleType::Bench;
    }
    else if (roll < 80) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_BOOK));
        obstacle.sprite.setScale(2.0f, 2.0f);
        spawnY = groundY - 100.0f;  // Flying book at head height
        obstacle.type = ObstacleType::Book;
    }
    else if (roll < 90) {
        // Professor obstacle - tall, slow
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_CHAIR));  // Reuse for now
        obstacle.sprite.setScale(5.0f, 6.0f);
        obstacle.speed *= 0.7f;  // Professors walk slow
        obstacle.type = ObstacleType::Professor;
    }
    else {
        // Coffee cart - wide
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_BENCH));  // Reuse for now
        obstacle.sprite.setScale(5.0f, 4.0f);
        obstacle.type = ObstacleType::CoffeeCart;
    }

    sf::FloatRect local = obstacle.sprite.getLocalBounds();
    obstacle.sprite.setOrigin(local.width * 0.5f, local.height);
    obstacle.sprite.setPosition(spawnX, spawnY);
    obstacles.push_back(obstacle);
}

void Game::updateObstacles(float dt) {
    float timeScale = powerUpManager->getSlowMoFactor();

    for (auto& obstacle : obstacles) {
        // Apply slow-mo to obstacle movement
        obstacle.sprite.move(-obstacle.speed * dt * timeScale, 0.0f);

        // Update animation timer for animated obstacles
        obstacle.animTimer += dt;

        // Bobbing animation for flying books
        if (obstacle.type == ObstacleType::Book) {
            float bob = std::sin(obstacle.animTimer * 4.0f) * 8.0f;
            sf::Vector2f pos = obstacle.sprite.getPosition();
            obstacle.sprite.setPosition(pos.x, groundY - 100.0f + bob);
        }

        // Check if passed player lane
        if (!obstacle.passed && obstacle.sprite.getPosition().x < laneX) {
            obstacle.passed = true;
            obstaclesDodged++;

            // Calculate score bonus
            int baseBonus = 20;
            float scoreMultiplier = powerUpManager->getScoreMultiplier();
            int finalBonus = comboSystem->calculateBonus(static_cast<int>(baseBonus * scoreMultiplier));
            score += finalBonus;

            // Register dodge with combo system
            comboSystem->registerDodge();

            // Show score popup
            hud->triggerScorePopup("+" + std::to_string(finalBonus), 
                sf::Vector2f(laneX + 50.0f, groundY - 100.0f));
        }
    }

    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(), [](const RunnerObstacle& obstacle) {
            sf::FloatRect bounds = obstacle.sprite.getGlobalBounds();
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

    for (const auto& obstacle : obstacles) {
        sf::FloatRect obstacleBounds = obstacle.sprite.getGlobalBounds();
        obstacleBounds.left += obstacleBounds.width * 0.12f;
        obstacleBounds.top += obstacleBounds.height * 0.12f;
        obstacleBounds.width *= 0.76f;
        obstacleBounds.height *= 0.82f;

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
            if (event.key.code == sf::Keyboard::Escape) {
                currentState = GameState::Pause;
                menuSystem->resetPauseSelection();
            }

            // Game over - handle restart/quit
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
            }
        }
    }
}

void Game::update(float dt) {
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
        // Handle pause menu input
        if (inputManager.isKeyPressed(sf::Keyboard::P) || inputManager.isKeyPressed(sf::Keyboard::Escape)) {
            currentState = GameState::Play;
        }
        if (inputManager.isKeyPressed(sf::Keyboard::Q)) {
            currentState = GameState::Menu;
            menuSystem->resetMenuState();
        }
        return;
    }

    if (!player || currentState != GameState::Play) {
        return;
    }

    if (gameOver) {
        if (inputManager.isKeyPressed(sf::Keyboard::Enter)) {
            resetRun();
        }
        if (inputManager.isKeyPressed(sf::Keyboard::Escape)) {
            currentState = GameState::Menu;
            menuSystem->resetMenuState();
        }
        particleSystem->update(dt);
        return;
    }

    // Get time scale from power-ups
    float timeScale = powerUpManager->getSlowMoFactor();
    float scaledDt = dt * timeScale;

    // Update run time and distance
    runTime += dt;
    distanceTraveled += obstacleBaseSpeed * dt * 0.1f;

    // Handle player input
    player->handleInput(inputManager);

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

    // Obstacle spawning - with maximum cap to prevent overcrowding
    constexpr size_t MAX_OBSTACLES_ON_SCREEN = 8;
    obstacleSpawnTimer += dt;
    if (obstacleSpawnTimer >= nextObstacleSpawnDelay && obstacles.size() < MAX_OBSTACLES_ON_SCREEN) {
        spawnObstacle();
        obstacleSpawnTimer = 0.0f;
        // Spawn delay scales down with speed but maintains a minimum gap
        float spawnScale = std::max(0.60f, 1.0f - (obstacleBaseSpeed - 430.0f) / 700.0f);
        nextObstacleSpawnDelay = randomRange(rng, 0.85f, 1.6f) * spawnScale;
    }

    // Update obstacles
    updateObstacles(scaledDt);

    // Update power-ups and coins
    powerUpManager->update(dt, obstacleBaseSpeed, rng);
    coinManager->update(dt, obstacleBaseSpeed, rng);

    // Check power-up collection
    if (player) {
        sf::FloatRect playerBounds = player->getBoundsScreen();
        if (powerUpManager->checkCollection(playerBounds)) {
            // Update HUD with active power-ups
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
                    default: name = "Power"; color = sf::Color::White;
                }
                hud->addPowerUp(name, color, effect.maxTime);
            }
            hud->showToast("Power-Up!", Constants::UI_ACCENT);
        }

        // Check coin collection
        bool hasMagnet = powerUpManager->hasMagnet();
        int coinsCollected = coinManager->checkCollection(playerBounds, hasMagnet, player->getScreenPosition());
        if (coinsCollected > 0) {
            coinsThisRun += coinsCollected;
            if (saveSystem) {
                saveSystem->addCoins(coinsCollected);
            }
            hud->triggerScorePopup("+" + std::to_string(coinsCollected) + " coin", player->getScreenPosition());
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

    // Update difficulty
    obstacleBaseSpeed = std::min(obstacleBaseSpeed + dt * Constants::DIFFICULTY_RAMP_RATE, Constants::MAX_OBSTACLE_SPEED);

    // Update visual systems
    particleSystem->update(dt);

    // Update HUD
    hud->setScore(score);
    hud->setBestScore(bestScore);
    hud->setCoins(coinsThisRun);
    hud->setCombo(comboSystem->getComboCount(), comboSystem->getMultiplier());
    hud->setSpeed(obstacleBaseSpeed, Constants::MAX_OBSTACLE_SPEED);
    hud->setDistance(distanceTraveled);
    hud->update(dt);

    // Update power-up display in HUD
    for (const auto& effect : powerUpManager->getActiveEffects()) {
        std::string name;
        switch (effect.type) {
            case PowerUpType::Shield: name = "Shield"; break;
            case PowerUpType::SlowMotion: name = "SlowMo"; break;
            case PowerUpType::DoubleScore: name = "Double"; break;
            case PowerUpType::Magnet: name = "Magnet"; break;
            default: name = "Power";
        }
        hud->updatePowerUp(name, effect.remainingTime);
    }

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
        } else {
            handleGameOver();
        }
    }

    // Check achievements
    checkAchievements();

    previousState = currentState;
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

    // Draw projectiles (bullets)
    projectileManager->render(window);

    // Particles
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
        projectileManager->fire(playerPos, mousePos);
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
            if (obstacle.hitPoints <= 0) continue;  // Already destroyed

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
                    case ObstacleType::CoffeeCart:
                        explosionColor = sf::Color(70, 50, 30);    // Coffee brown
                        break;
                    default:
                        explosionColor = sf::Color(150, 130, 100);
                }

                if (obstacle.hitPoints <= 0) {
                    // Obstacle destroyed - spawn big explosion!
                    particleSystem->spawnExplosion(obstacleCenter, explosionColor, 30);
                    
                    // Add some bright sparks
                    particleSystem->spawnExplosion(obstacleCenter, sf::Color(255, 200, 100), 15);

                    // Give bonus points
                    int destroyBonus = 50;
                    float scoreMultiplier = powerUpManager->getScoreMultiplier();
                    int finalBonus = static_cast<int>(destroyBonus * scoreMultiplier);
                    score += finalBonus;

                    // Show destruction popup
                    hud->triggerScorePopup("DESTROYED! +" + std::to_string(finalBonus),
                        sf::Vector2f(obstacle.sprite.getPosition().x, 
                                     obstacle.sprite.getPosition().y - 50.0f));
                    hud->showToast("Object destroyed!", sf::Color(255, 200, 100));

                    // Bigger screen shake for destruction
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

    // Remove destroyed obstacles
    obstacles.erase(
        std::remove_if(obstacles.begin(), obstacles.end(), [](const RunnerObstacle& o) {
            return o.hitPoints <= 0;
        }),
        obstacles.end()
    );
}
