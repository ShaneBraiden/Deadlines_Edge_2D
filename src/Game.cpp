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
    , currentState(GameState::Menu)
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
{
    window.setFramerateLimit(60);

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Lab: single-level pointer + exception handling
    // AssetManager::loadAll() throws AssetLoadException on failure,
    // which propagates to main.cpp's try/catch
    assets = new AssetManager();
    assets->loadAll();

    physics = new PhysicsWorld(winW, winH);
    particleSystem = new ParticleSystem(winW, winH);
    parallaxBg = new ParallaxBackground();

    // Lab: arrow operator + template method call
    parallaxBg->addLayer(assets->get<sf::Texture>(AssetKeys::BG_FAR), 0.10f);
    parallaxBg->addLayer(assets->get<sf::Texture>(AssetKeys::BG_MID), 0.28f);
    parallaxBg->addLayer(assets->get<sf::Texture>(AssetKeys::BG_NEAR), 0.54f);

    lightingSystem = new LightingSystem(
        static_cast<unsigned int>(winW),
        static_cast<unsigned int>(winH)
    );
    lightingSystem->setAmbientLevel(190);

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
    for (auto* entity : entities) {
        delete entity;
    }
    entities.clear();

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
    const float groundHalfHeightMeters = 1.0f;  // 1 meter thick (50 pixels)
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
    // Position player so their feet are slightly above ground, then let them drop
    sf::Texture& playerTex = assets->get<sf::Texture>(AssetKeys::PLAYER_SHEET);
    float playerHalfHeightPixels = Constants::PLAYER_HEIGHT * Constants::PPM * 0.5f;
    float playerCenterScreenY = groundY - playerHalfHeightPixels - 5.0f;  // 5 pixels above ground
    b2Vec2 playerSpawn = physics->toWorld(sf::Vector2f(laneX, playerCenterScreenY));
    player = new Player(physics, playerSpawn.x, playerSpawn.y, playerTex);
    player->setRunnerMode(true);
    entities.push_back(player);

    obstacleSpawnTimer = 0.0f;
    nextObstacleSpawnDelay = 1.1f;
    obstacleBaseSpeed = 430.0f;
    score = 0.0f;
    gameOver = false;
}

void Game::spawnObstacle() {
    std::uniform_int_distribution<int> typeDist(0, 99);
    int roll = typeDist(rng);

    RunnerObstacle obstacle;
    obstacle.speed = obstacleBaseSpeed + randomRange(rng, -25.0f, 90.0f);
    obstacle.passed = false;

    float spawnX = static_cast<float>(window.getSize().x) + randomRange(rng, 60.0f, 260.0f);
    float spawnY = groundY;

    // Lab: arrow operator + template method call
    if (roll < 45) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_CHAIR));
        obstacle.sprite.setScale(4.5f, 4.5f);  // Bigger obstacles for bigger player
    }
    else if (roll < 80) {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_BENCH));
        obstacle.sprite.setScale(3.2f, 3.2f);
    }
    else {
        obstacle.sprite.setTexture(assets->get<sf::Texture>(AssetKeys::OBSTACLE_BOOK));
        obstacle.sprite.setScale(2.0f, 2.0f);  // Smaller flying book
        // Flying book at head height — duck to avoid, don't jump
        spawnY = groundY - 100.0f;
    }

    sf::FloatRect local = obstacle.sprite.getLocalBounds();
    obstacle.sprite.setOrigin(local.width * 0.5f, local.height);
    obstacle.sprite.setPosition(spawnX, spawnY);
    obstacles.push_back(obstacle);
}

void Game::updateObstacles(float dt) {
    for (auto& obstacle : obstacles) {
        obstacle.sprite.move(-obstacle.speed * dt, 0.0f);
        if (!obstacle.passed && obstacle.sprite.getPosition().x < laneX) {
            obstacle.passed = true;
            score += 20.0f;
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

bool Game::checkObstacleCollision() const {
    if (!player) {
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

        if (currentState == GameState::Menu && event.key.code == sf::Keyboard::Enter) {
            startGameFromMenu();
            continue;
        }

        if (event.key.code == sf::Keyboard::Escape) {
            if (currentState == GameState::Play) {
                currentState = GameState::Pause;
            }
            else if (currentState == GameState::Pause) {
                currentState = GameState::Play;
            }
            else {
                window.close();
            }
        }

        if (currentState == GameState::Play && gameOver && event.key.code == sf::Keyboard::Enter) {
            resetRun();
        }
    }
}

void Game::update(float dt) {
    if (currentState == GameState::Menu) {
        if (inputManager.isKeyPressed(sf::Keyboard::Enter)) {
            startGameFromMenu();
        }
        particleSystem->update(dt);
        return;
    }

    if (currentState == GameState::Pause) {
        if (inputManager.isKeyPressed(sf::Keyboard::P)) {
            currentState = GameState::Play;
        }
        if (inputManager.isKeyPressed(sf::Keyboard::Q)) {
            currentState = GameState::Menu;
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
        particleSystem->update(dt);
        return;
    }

    player->handleInput(inputManager);

    for (auto* entity : entities) {
        entity->update(dt);
    }

    physics->step();

    b2Body* playerBody = player->getBody();
    b2Vec2 velocity = playerBody->GetLinearVelocity();
    velocity.x = 0.0f;
    playerBody->SetLinearVelocity(velocity);

    b2Vec2 playerPos = playerBody->GetPosition();
    float laneXWorld = physics->toWorld(sf::Vector2f(laneX, 0.0f)).x;
    playerBody->SetTransform(b2Vec2(laneXWorld, playerPos.y), 0.0f);

    obstacleSpawnTimer += dt;
    if (obstacleSpawnTimer >= nextObstacleSpawnDelay) {
        spawnObstacle();
        obstacleSpawnTimer = 0.0f;
        float spawnScale = std::max(0.55f, 1.0f - (obstacleBaseSpeed - 430.0f) / 600.0f);
        nextObstacleSpawnDelay = randomRange(rng, 0.7f, 1.5f) * spawnScale;
    }

    updateObstacles(dt);

    score += dt * 14.0f;
    obstacleBaseSpeed = std::min(obstacleBaseSpeed + dt * 8.0f, 830.0f);
    particleSystem->update(dt);

    if (checkObstacleCollision()) {
        gameOver = true;
        bestScore = std::max(bestScore, score);
    }
}

void Game::render() {
    window.clear(sf::Color(8, 10, 15));
    window.setView(window.getDefaultView());

    if (parallaxBg) {
        parallaxBg->render(window, window.getView());
    }

    if (currentState == GameState::Menu) {
        renderMenu();
        window.display();
        return;
    }

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    sf::RectangleShape groundBand;
    groundBand.setSize(sf::Vector2f(winW, winH - groundY));
    groundBand.setPosition(0.0f, groundY);
    groundBand.setFillColor(sf::Color(18, 22, 32, 220));
    window.draw(groundBand);

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

    for (auto* entity : entities) {
        entity->render(window);
    }

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
    }

    if (particleSystem) {
        particleSystem->render(window);
    }

    if (lightingSystem && player) {
        lightingSystem->clearLights();
        b2Vec2 bodyPos = player->getBody()->GetPosition();
        sf::Vector2f screenPos = physics->toScreen(bodyPos);
        lightingSystem->addLight(
            screenPos,
            260.0f,
            sf::Color(254, 225, 190),
            0.55f,
            0.18f
        );
        lightingSystem->render(window, Constants::TIME_STEP);
    }

    renderVignette();

    // Lab: arrow operator + template method call
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);

    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setCharacterSize(30);
    scoreText.setFillColor(sf::Color(230, 235, 245));
    scoreText.setString("Score  " + std::to_string(static_cast<int>(score)));
    scoreText.setPosition(28.0f, 26.0f);
    window.draw(scoreText);

    sf::Text bestText;
    bestText.setFont(font);
    bestText.setCharacterSize(16);
    bestText.setFillColor(sf::Color(160, 172, 195));
    bestText.setString("Best  " + std::to_string(static_cast<int>(bestScore)));
    bestText.setPosition(32.0f, 66.0f);
    window.draw(bestText);

    sf::Text hintText;
    hintText.setFont(font);
    hintText.setCharacterSize(14);
    hintText.setFillColor(sf::Color(150, 160, 178, 190));
    hintText.setString("SPACE / W / UP to jump  |  S / DOWN to duck");
    hintText.setPosition(30.0f, winH - 42.0f);
    window.draw(hintText);

    if (currentState == GameState::Pause) {
        renderPause();
    }

    if (gameOver) {
        renderGameOver();
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
