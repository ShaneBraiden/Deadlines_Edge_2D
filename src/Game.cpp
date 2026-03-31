#include "Game.h"
#include "Constants.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <filesystem>
#include <iostream>

namespace {

void fillRect(sf::Image& image, int x, int y, int w, int h, const sf::Color& color) {
    for (int py = y; py < y + h; ++py) {
        for (int px = x; px < x + w; ++px) {
            if (px >= 0 && py >= 0 && px < static_cast<int>(image.getSize().x) && py < static_cast<int>(image.getSize().y)) {
                image.setPixel(static_cast<unsigned int>(px), static_cast<unsigned int>(py), color);
            }
        }
    }
}

void drawStudentFrame(sf::Image& image, int frameX, int frameY, int legOffset, bool jump, bool fall) {
    const sf::Color skin(221, 187, 156);
    const sf::Color hair(54, 38, 28);
    const sf::Color hoodie(50, 65, 90);
    const sf::Color hoodieLight(65, 82, 110);
    const sf::Color pant(28, 36, 56);
    const sf::Color shoe(230, 230, 230);
    const sf::Color bag(86, 45, 30);
    const sf::Color bagStrap(70, 38, 25);
    const sf::Color outline(20, 20, 30);

    // SIDE PROFILE view - student running to the right
    const int s = 3;  // Scale factor
    int bx = frameX + 30;  // Base X (centered in 128px frame)
    int by = frameY + 8;   // Base Y

    // --- HEAD (side profile - oval shape) ---
    // Hair (back of head, drawn first)
    fillRect(image, bx + 8*s, by + 2*s, 6*s, 10*s, hair);
    // Head outline
    fillRect(image, bx + 10*s - 1, by + 3*s - 1, 8*s + 2, 9*s + 2, outline);
    // Face (side, narrower)
    fillRect(image, bx + 10*s, by + 3*s, 8*s, 9*s, skin);
    // Hair on top
    fillRect(image, bx + 9*s, by + 1*s, 8*s, 4*s, hair);
    // Eye (single, side view)
    fillRect(image, bx + 15*s, by + 5*s, 2*s, 2*s, outline);
    // Ear
    fillRect(image, bx + 9*s, by + 6*s, 2*s, 3*s, skin);

    int torsoLift = jump ? -2*s : (fall ? 1*s : 0);

    // --- BACKPACK (behind body) ---
    fillRect(image, bx + 4*s - 1, by + 13*s + torsoLift - 1, 7*s + 2, 12*s + 2, outline);
    fillRect(image, bx + 4*s, by + 13*s + torsoLift, 7*s, 12*s, bag);
    // Backpack strap
    fillRect(image, bx + 10*s, by + 14*s + torsoLift, 2*s, 8*s, bagStrap);

    // --- TORSO (side view - narrower) ---
    fillRect(image, bx + 9*s - 1, by + 12*s + torsoLift - 1, 10*s + 2, 14*s + 2, outline);
    fillRect(image, bx + 9*s, by + 12*s + torsoLift, 10*s, 14*s, hoodie);
    // Hoodie highlight
    fillRect(image, bx + 14*s, by + 13*s + torsoLift, 4*s, 12*s, hoodieLight);

    // --- ARMS (pumping motion for running) ---
    int armSwing = legOffset * s;
    // Back arm
    fillRect(image, bx + 6*s - 1, by + 14*s + torsoLift - armSwing - 1, 4*s + 2, 8*s + 2, outline);
    fillRect(image, bx + 6*s, by + 14*s + torsoLift - armSwing, 4*s, 8*s, skin);
    // Front arm
    fillRect(image, bx + 17*s - 1, by + 14*s + torsoLift + armSwing - 1, 4*s + 2, 8*s + 2, outline);
    fillRect(image, bx + 17*s, by + 14*s + torsoLift + armSwing, 4*s, 8*s, skin);

    // --- LEGS (running motion) ---
    int frontLegX = bx + 14*s;
    int backLegX = bx + 9*s;
    int legBaseY = by + 26*s + torsoLift;

    int frontLegOffset = jump ? 2*s : (fall ? -1*s : legOffset * s);
    int backLegOffset = jump ? -2*s : (fall ? 1*s : -legOffset * s);

    // Back leg
    fillRect(image, backLegX - 1, legBaseY + backLegOffset - 1, 5*s + 2, 10*s + 2, outline);
    fillRect(image, backLegX, legBaseY + backLegOffset, 5*s, 10*s, pant);
    // Back shoe
    fillRect(image, backLegX - 1*s - 1, legBaseY + 9*s + backLegOffset - 1, 7*s + 2, 3*s + 2, outline);
    fillRect(image, backLegX - 1*s, legBaseY + 9*s + backLegOffset, 7*s, 3*s, shoe);

    // Front leg
    fillRect(image, frontLegX - 1, legBaseY + frontLegOffset - 1, 5*s + 2, 10*s + 2, outline);
    fillRect(image, frontLegX, legBaseY + frontLegOffset, 5*s, 10*s, pant);
    // Front shoe
    fillRect(image, frontLegX - 1, legBaseY + 9*s + frontLegOffset - 1, 7*s + 2, 3*s + 2, outline);
    fillRect(image, frontLegX, legBaseY + 9*s + frontLegOffset, 7*s, 3*s, shoe);
}

void generateStudentSheet(const std::string& path) {
    sf::Image image;
    image.create(1024, 512, sf::Color(0, 0, 0, 0));

    for (int f = 0; f < 8; ++f) {
        int step = (f % 4) - 2;
        drawStudentFrame(image, f * 128, 0, step, false, false);
        drawStudentFrame(image, f * 128, 128, step, false, false);
        drawStudentFrame(image, f * 128, 256, 0, true, false);
        drawStudentFrame(image, f * 128, 384, 0, false, true);
    }

    image.saveToFile(path);
}

void generateChair(const std::string& path) {
    sf::Image image;
    image.create(32, 32, sf::Color(0, 0, 0, 0));
    fillRect(image, 6, 5, 20, 4, sf::Color(80, 55, 38));
    fillRect(image, 8, 9, 3, 15, sf::Color(70, 45, 30));
    fillRect(image, 21, 9, 3, 15, sf::Color(70, 45, 30));
    fillRect(image, 6, 20, 20, 4, sf::Color(96, 65, 45));
    fillRect(image, 9, 24, 3, 7, sf::Color(60, 40, 26));
    fillRect(image, 20, 24, 3, 7, sf::Color(60, 40, 26));
    image.saveToFile(path);
}

void generateBench(const std::string& path) {
    sf::Image image;
    image.create(64, 32, sf::Color(0, 0, 0, 0));
    fillRect(image, 4, 9, 56, 4, sf::Color(110, 78, 52));
    fillRect(image, 4, 17, 56, 4, sf::Color(120, 86, 58));
    fillRect(image, 10, 21, 4, 10, sf::Color(74, 52, 36));
    fillRect(image, 50, 21, 4, 10, sf::Color(74, 52, 36));
    fillRect(image, 26, 21, 4, 10, sf::Color(74, 52, 36));
    fillRect(image, 34, 21, 4, 10, sf::Color(74, 52, 36));
    image.saveToFile(path);
}

void generateBook(const std::string& path) {
    sf::Image image;
    image.create(28, 20, sf::Color(0, 0, 0, 0));
    fillRect(image, 2, 2, 24, 15, sf::Color(145, 42, 42));
    fillRect(image, 4, 4, 20, 11, sf::Color(176, 57, 57));
    fillRect(image, 12, 2, 2, 15, sf::Color(240, 228, 190));
    fillRect(image, 3, 17, 22, 2, sf::Color(230, 220, 180));
    image.saveToFile(path);
}

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
    , obstacleSpawnTimer(0.0f)
    , nextObstacleSpawnDelay(1.2f)
    , obstacleBaseSpeed(430.0f)
    , score(0.0f)
    , bestScore(0.0f)
    , gameOver(false)
    , rng(static_cast<unsigned int>(std::time(nullptr)))
    , chairTexture(nullptr)
    , benchTexture(nullptr)
    , bookTexture(nullptr)
    , laneX(0.0f)
    , groundY(0.0f)
{
    window.setFramerateLimit(60);

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    ensureGeneratedAssets();

    physics = new PhysicsWorld(winW, winH);
    particleSystem = new ParticleSystem(winW, winH);
    parallaxBg = new ParallaxBackground();

    try {
        sf::Texture& bgFar = textures.get("bg_far", "assets/textures/bg_far.png");
        sf::Texture& bgMid = textures.get("bg_mid", "assets/textures/bg_mid.png");
        sf::Texture& bgNear = textures.get("bg_near", "assets/textures/bg_near.png");
        bgFar.setRepeated(true);
        bgMid.setRepeated(true);
        bgNear.setRepeated(true);
        parallaxBg->addLayer(bgFar, 0.10f);
        parallaxBg->addLayer(bgMid, 0.28f);
        parallaxBg->addLayer(bgNear, 0.54f);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "[Game] Parallax textures not loaded: " << e.what() << std::endl;
    }

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

    fonts.preload("main", "assets/fonts/main.ttf");
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
}

void Game::ensureGeneratedAssets() {
    namespace fs = std::filesystem;
    fs::create_directories("assets/textures");

    const std::string studentPath = "assets/textures/student_runner.png";
    const std::string chairPath = "assets/textures/obstacle_chair.png";
    const std::string benchPath = "assets/textures/obstacle_bench.png";
    const std::string bookPath = "assets/textures/obstacle_book.png";

    if (!fs::exists(studentPath)) generateStudentSheet(studentPath);
    if (!fs::exists(chairPath)) generateChair(chairPath);
    if (!fs::exists(benchPath)) generateBench(benchPath);
    if (!fs::exists(bookPath)) generateBook(bookPath);
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

    // Position player so their feet are slightly above ground, then let them drop
    sf::Texture& playerTex = textures.get("player_runner", "assets/textures/student_runner.png");
    float playerHalfHeightPixels = Constants::PLAYER_HEIGHT * Constants::PPM * 0.5f;
    float playerCenterScreenY = groundY - playerHalfHeightPixels - 5.0f;  // 5 pixels above ground
    b2Vec2 playerSpawn = physics->toWorld(sf::Vector2f(laneX, playerCenterScreenY));
    player = new Player(physics, playerSpawn.x, playerSpawn.y, playerTex);
    player->setRunnerMode(true);
    entities.push_back(player);

    chairTexture = &textures.get("ob_chair", "assets/textures/obstacle_chair.png");
    benchTexture = &textures.get("ob_bench", "assets/textures/obstacle_bench.png");
    bookTexture = &textures.get("ob_book", "assets/textures/obstacle_book.png");

    obstacleSpawnTimer = 0.0f;
    nextObstacleSpawnDelay = 1.1f;
    obstacleBaseSpeed = 430.0f;
    score = 0.0f;
    gameOver = false;
}

void Game::spawnObstacle() {
    if (!chairTexture || !benchTexture || !bookTexture) {
        return;
    }

    std::uniform_int_distribution<int> typeDist(0, 99);
    int roll = typeDist(rng);

    RunnerObstacle obstacle;
    obstacle.speed = obstacleBaseSpeed + randomRange(rng, -25.0f, 90.0f);
    obstacle.passed = false;

    float spawnX = static_cast<float>(window.getSize().x) + randomRange(rng, 60.0f, 260.0f);
    float spawnY = groundY;

    if (roll < 45) {
        obstacle.sprite.setTexture(*chairTexture);
        obstacle.sprite.setScale(4.5f, 4.5f);  // Bigger obstacles for bigger player
    }
    else if (roll < 80) {
        obstacle.sprite.setTexture(*benchTexture);
        obstacle.sprite.setScale(3.2f, 3.2f);
    }
    else {
        obstacle.sprite.setTexture(*bookTexture);
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

    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

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
    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

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
    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

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
    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

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
