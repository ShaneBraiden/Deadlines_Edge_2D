#include "Game.h"
#include "Constants.h"
#include <iostream>
#include <algorithm>
#include <cmath>

Game::Game()
    : window(sf::VideoMode::getDesktopMode(), "Deadline's Edge", sf::Style::Fullscreen)
    , physics(nullptr)
    , player(nullptr)
    , tileMap(nullptr)
    , particleSystem(nullptr)
    , parallaxBg(nullptr)
    , lightingSystem(nullptr)
    , currentState(GameState::Menu)
    , currentLevel(0)
    , endingTimer(0.0f)
    , endingFadeAlpha(0.0f)
    , endingTriggered(false)
{
    window.setFramerateLimit(60);

    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Level file paths
    levelFiles[0] = "assets/levels/level1.txt";
    levelFiles[1] = "assets/levels/level2.txt";
    levelFiles[2] = "assets/levels/level3.txt";

    // Create physics world -- Lab: single-level pointer via new
    physics = new PhysicsWorld(winW, winH);

    // Create tile map -- Lab: single-level pointer via new
    tileMap = new TileMap();

    // Create particle system -- Lab: single-level pointer via new
    particleSystem = new ParticleSystem(winW, winH);

    // Create parallax background -- Lab: single-level pointer via new
    parallaxBg = new ParallaxBackground();
    try {
        sf::Texture& bgFar  = textures.get("bg_far",  "assets/textures/bg_far.png");
        sf::Texture& bgMid  = textures.get("bg_mid",  "assets/textures/bg_mid.png");
        sf::Texture& bgNear = textures.get("bg_near", "assets/textures/bg_near.png");
        bgFar.setRepeated(true);
        bgMid.setRepeated(true);
        bgNear.setRepeated(true);
        parallaxBg->addLayer(bgFar,  0.1f);   // Farthest: dark sky/building exterior
        parallaxBg->addLayer(bgMid,  0.3f);   // Mid: blurred hallway depth
        parallaxBg->addLayer(bgNear, 0.6f);   // Near: foreground fog/details
    }
    catch (const std::runtime_error& e) {
        std::cerr << "[Game] Parallax textures not found: " << e.what() << std::endl;
    }

    // Create lighting system -- Lab: single-level pointer via new
    lightingSystem = new LightingSystem(
        static_cast<unsigned int>(winW),
        static_cast<unsigned int>(winH)
    );
    lightingSystem->setAmbientLevel(200);  // TODO: lower to ~25 when real art is added

    // Setup vignette overlay for horror atmosphere
    float vigSize = 120.0f;
    vignetteTop.setSize(sf::Vector2f(winW, vigSize));
    vignetteTop.setPosition(0.0f, 0.0f);
    vignetteTop.setFillColor(sf::Color(0, 0, 0, 100));

    vignetteBottom.setSize(sf::Vector2f(winW, vigSize));
    vignetteBottom.setPosition(0.0f, winH - vigSize);
    vignetteBottom.setFillColor(sf::Color(0, 0, 0, 100));

    vignetteLeft.setSize(sf::Vector2f(vigSize, winH));
    vignetteLeft.setPosition(0.0f, 0.0f);
    vignetteLeft.setFillColor(sf::Color(0, 0, 0, 80));

    vignetteRight.setSize(sf::Vector2f(vigSize, winH));
    vignetteRight.setPosition(winW - vigSize, 0.0f);
    vignetteRight.setFillColor(sf::Color(0, 0, 0, 80));

    // Load font -- Lab: ResourceManager<sf::Font> template usage, exception handling
    try {
        fonts.preload("main", "assets/fonts/main.ttf");
    }
    catch (const std::runtime_error& e) {
        std::cerr << "[Game] Font load failed: " << e.what() << std::endl;
        throw;  // Font is required for UI -- cannot continue
    }

    // Demonstrate ResourceManager exception handling -- Lab: try/catch, rethrowing
    try {
        textures.preload("placeholder", "assets/textures/placeholder.png");
    }
    catch (const std::runtime_error& e) {
        // Lab: exception handling -- catch the rethrown exception from ResourceManager
        std::cerr << "[Game] Resource preload failed (expected): "
                  << e.what() << std::endl;
    }
}

Game::~Game() {
    clearLevel();

    delete lightingSystem;
    lightingSystem = nullptr;

    delete parallaxBg;
    parallaxBg = nullptr;

    delete particleSystem;
    particleSystem = nullptr;

    delete tileMap;
    tileMap = nullptr;

    delete physics;
    physics = nullptr;
}

void Game::startGameFromMenu() {
    currentLevel = 0;
    endingTriggered = false;
    endingTimer = 0.0f;
    endingFadeAlpha = 0.0f;
    loadLevel(0);
    currentState = GameState::Play;
}

void Game::applyGameplayView() {
    sf::View gameplayView(sf::FloatRect(
        0.0f,
        0.0f,
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)
    ));

    if (!player || !tileMap || tileMap->getRows() <= 0 || tileMap->getCols() <= 0) {
        window.setView(gameplayView);
        return;
    }

    b2Vec2 playerWorld = player->getBody()->GetPosition();
    sf::Vector2f playerScreen = physics->toScreen(playerWorld);

    float levelPixelWidth = tileMap->getCols() * tileMap->getTileSize() * Constants::PPM;
    float levelPixelHeight = tileMap->getRows() * tileMap->getTileSize() * Constants::PPM;

    float viewW = gameplayView.getSize().x;
    float viewH = gameplayView.getSize().y;
    float halfW = viewW * 0.5f;
    float halfH = viewH * 0.5f;

    float minX = halfW;
    float maxX = std::max(halfW, levelPixelWidth - halfW);
    float minY = halfH;
    float maxY = std::max(halfH, levelPixelHeight - halfH);

    float clampedX = std::max(minX, std::min(playerScreen.x, maxX));
    float clampedY = std::max(minY, std::min(playerScreen.y, maxY));

    gameplayView.setCenter(clampedX, clampedY);
    window.setView(gameplayView);
}

void Game::clearLevel() {
    // Delete all entities -- Lab: STL vector iteration, pointer cleanup
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        delete *it;    // Lab: STL iterator dereference, pointer delete
    }
    entities.clear();
    remnants.clear();  // Non-owning pointers (already deleted above)
    player = nullptr;

    // Clear tile map (frees Tile** grid)
    if (tileMap) {
        tileMap->clear();
    }

    // Recreate physics world to clear all bodies
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    delete physics;
    physics = new PhysicsWorld(winW, winH);
}

void Game::loadLevel(int levelIndex) {
    clearLevel();
    currentLevel = levelIndex;

    // Load tile map from file -- Lab: exception handling
    try {
        tileMap->loadFromFile(levelFiles[levelIndex]);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "[Game] Failed to load level " << levelIndex << ": "
                  << e.what() << std::endl;
        // Lab: rethrowing -- propagate to caller
        throw;
    }

    // Create Box2D bodies for solid tiles
    tileMap->createBodies(physics);

    // Spawn player at the level's spawn point
    sf::Vector2f spawn = tileMap->getPlayerSpawn();
    sf::Texture& playerTex = textures.get("player", "assets/textures/player.png");
    player = new Player(physics, spawn.x, spawn.y, playerTex);
    entities.push_back(player);

    // Spawn Remnants at marked positions
    sf::Texture& remnantTex = textures.get("remnant", "assets/textures/remnant.png");
    const auto& remnantPositions = tileMap->getRemnantSpawns();
    for (const auto& pos : remnantPositions) {
        Remnant* remnant = new Remnant(physics, pos.x, pos.y, remnantTex);
        remnants.push_back(remnant);
        entities.push_back(remnant);
    }
}

void Game::checkLevelTransition() {
    if (!player || !tileMap) return;

    // Get player position in world coords
    b2Vec2 playerPos = player->getBody()->GetPosition();
    sf::Vector2f exitPos = tileMap->getExitPosition();

    // Check distance to exit
    float dx = playerPos.x - exitPos.x;
    float dy = playerPos.y - exitPos.y;
    float dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 1.5f) {  // Within 1.5 meters of exit
        if (currentLevel < TOTAL_LEVELS - 1) {
            // Next level
            loadLevel(currentLevel + 1);
        }
        else {
            // Final level complete -- trigger ending
            endingTriggered = true;
            currentState = GameState::Menu;  // Reuse menu state for ending
        }
    }
}

void Game::run() {
    sf::Clock clock;
    float accumulator = 0.0f;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();
        if (dt > 0.25f) dt = 0.25f;

        accumulator += dt;

        processEvents();
        inputManager.update();

        // Update ending timer if triggered
        if (endingTriggered) {
            endingTimer += dt;
            endingFadeAlpha = std::min(endingTimer * 50.0f, 255.0f);
        }

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
        }

        if (event.type == sf::Event::KeyPressed) {
            if (currentState == GameState::Menu && !endingTriggered &&
                event.key.code == sf::Keyboard::Enter) {
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
                else if (currentState == GameState::Menu) {
                    if (endingTriggered) {
                        window.close();
                    }
                }
            }
        }
    }
}

void Game::update(float dt) {
    // Menu: wait for Enter to start
    if (currentState == GameState::Menu && !endingTriggered) {
        // Fallback polling path in case KeyPressed was missed.
        if (inputManager.isKeyPressed(sf::Keyboard::Enter)) {
            startGameFromMenu();
        }
        return;
    }

    // Pause: wait for P or Escape to resume
    if (currentState == GameState::Pause) {
        if (inputManager.isKeyPressed(sf::Keyboard::P)) {
            currentState = GameState::Play;
        }
        if (inputManager.isKeyPressed(sf::Keyboard::Q)) {
            currentState = GameState::Menu;
            clearLevel();
            endingTriggered = false;
            endingTimer = 0.0f;
        }
        return;
    }

    if (currentState != GameState::Play) return;

    player->handleInput(inputManager);

    // Update Remnants with player position
    if (player) {
        b2Vec2 bodyPos = player->getBody()->GetPosition();
        sf::Vector2f playerScreen = physics->toScreen(bodyPos);
        for (auto it = remnants.begin(); it != remnants.end(); ++it) {
            (*it)->setPlayerPosition(playerScreen);
        }
    }

    // Update all entities -- Lab: STL vector, STL iterators, arrow operator
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        (*it)->update(dt);
    }

    physics->step();

    // Update particle system -- Lab: STL list (internally)
    particleSystem->update(dt);

    // Check if player reached the exit
    checkLevelTransition();
}

void Game::render() {
    window.clear(sf::Color(10, 10, 12));
    window.setView(window.getDefaultView());

    if (currentState == GameState::Menu && !endingTriggered) {
        renderMenu();
        window.display();
        return;
    }

    if (endingTriggered) {
        renderEnding();
        window.display();
        return;
    }

    applyGameplayView();

    // Draw parallax background layers (behind everything)
    if (parallaxBg) {
        parallaxBg->render(window, window.getView());
    }

    // Draw tile map
    if (tileMap) {
        tileMap->render(window, physics);
    }

    // Draw all entities -- Lab: STL vector, STL iterators, arrow operator
    for (auto it = entities.begin(); it != entities.end(); ++it) {
        (*it)->render(window);
    }

    // Draw particles -- Lab: STL list (internally)
    particleSystem->render(window);

    // Save gameplay view for coordinate mapping, then switch to screen space
    sf::View gameplayView = window.getView();
    window.setView(window.getDefaultView());

    // Dynamic lighting pass (composited in screen space)
    if (lightingSystem) {
        lightingSystem->clearLights();

        // Player light: warm flickering glow (lantern/phone screen)
        if (player) {
            b2Vec2 bodyPos = player->getBody()->GetPosition();
            sf::Vector2f worldPos = physics->toScreen(bodyPos);
            sf::Vector2i pixelPos = window.mapCoordsToPixel(worldPos, gameplayView);
            lightingSystem->addLight(
                sf::Vector2f(static_cast<float>(pixelPos.x), static_cast<float>(pixelPos.y)),
                300.0f,
                sf::Color(255, 220, 180),  // Warm light
                0.8f,
                0.3f                        // Flicker
            );
        }

        // Remnant cold glow -- Lab: STL vector iteration with STL iterators
        for (auto it = remnants.begin(); it != remnants.end(); ++it) {
            float dist = (*it)->getDistanceToPlayer();
            if (dist < Remnant::DETECTION_RANGE) {
                b2Vec2 remBody = (*it)->getBody()->GetPosition();
                sf::Vector2f remWorld = physics->toScreen(remBody);
                sf::Vector2i remPixel = window.mapCoordsToPixel(remWorld, gameplayView);
                float intensity = 0.3f * (1.0f - dist / Remnant::DETECTION_RANGE);
                lightingSystem->addLight(
                    sf::Vector2f(static_cast<float>(remPixel.x), static_cast<float>(remPixel.y)),
                    150.0f,
                    sf::Color(100, 120, 200),  // Cold blue
                    intensity,
                    0.1f
                );
            }
        }

        lightingSystem->render(window, Constants::TIME_STEP);
    }

    // Draw vignette overlay
    renderVignette();

    // Draw pause overlay if paused
    if (currentState == GameState::Pause) {
        renderPause();
    }

    // HUD: level indicator
    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

    sf::RectangleShape levelBar;
    float barWidth = 150.0f;
    float barHeight = 4.0f;
    float progress = static_cast<float>(currentLevel + 1) / static_cast<float>(TOTAL_LEVELS);
    levelBar.setSize(sf::Vector2f(barWidth * progress, barHeight));
    levelBar.setPosition(20.0f, 32.0f);
    levelBar.setFillColor(sf::Color(60, 80, 100, 120));
    window.draw(levelBar);

    sf::RectangleShape levelBarBg;
    levelBarBg.setSize(sf::Vector2f(barWidth, barHeight));
    levelBarBg.setPosition(20.0f, 32.0f);
    levelBarBg.setFillColor(sf::Color::Transparent);
    levelBarBg.setOutlineColor(sf::Color(40, 40, 50, 80));
    levelBarBg.setOutlineThickness(1.0f);
    window.draw(levelBarBg);

    sf::Text levelLabel;
    levelLabel.setFont(font);
    levelLabel.setString("Level " + std::to_string(currentLevel + 1) + " / " + std::to_string(TOTAL_LEVELS));
    levelLabel.setCharacterSize(12);
    levelLabel.setFillColor(sf::Color(70, 80, 100, 140));
    levelLabel.setPosition(20.0f, 12.0f);
    window.draw(levelLabel);

    window.display();
}

void Game::renderMenu() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("DEADLINE'S  EDGE");
    title.setCharacterSize(54);
    title.setFillColor(sf::Color(180, 190, 210));
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.left + titleBounds.width / 2.0f,
                    titleBounds.top + titleBounds.height / 2.0f);
    title.setPosition(winW / 2.0f, winH / 3.0f);
    window.draw(title);

    // Tagline
    sf::Text tagline;
    tagline.setFont(font);
    tagline.setString("Some deadlines follow you home.");
    tagline.setCharacterSize(16);
    tagline.setFillColor(sf::Color(80, 90, 110, 180));
    sf::FloatRect tagBounds = tagline.getLocalBounds();
    tagline.setOrigin(tagBounds.left + tagBounds.width / 2.0f,
                      tagBounds.top + tagBounds.height / 2.0f);
    tagline.setPosition(winW / 2.0f, winH / 3.0f + 55.0f);
    window.draw(tagline);

    // Decorative line under tagline
    sf::RectangleShape line;
    line.setSize(sf::Vector2f(250.0f, 1.0f));
    line.setOrigin(125.0f, 0.5f);
    line.setPosition(winW / 2.0f, winH / 3.0f + 80.0f);
    line.setFillColor(sf::Color(50, 60, 80, 120));
    window.draw(line);

    // "Press Enter" indicator (blinking)
    float time = static_cast<float>(std::clock()) / CLOCKS_PER_SEC;
    float alpha = (std::sin(time * 3.0f) + 1.0f) * 0.5f * 150.0f + 40.0f;
    sf::Text pressEnter;
    pressEnter.setFont(font);
    pressEnter.setString("Press  Enter");
    pressEnter.setCharacterSize(18);
    pressEnter.setFillColor(sf::Color(120, 130, 150, static_cast<int>(alpha)));
    sf::FloatRect enterBounds = pressEnter.getLocalBounds();
    pressEnter.setOrigin(enterBounds.left + enterBounds.width / 2.0f,
                         enterBounds.top + enterBounds.height / 2.0f);
    pressEnter.setPosition(winW / 2.0f, winH * 0.65f);
    window.draw(pressEnter);

    // Controls hint
    sf::Text controls;
    controls.setFont(font);
    controls.setString("ESC to quit");
    controls.setCharacterSize(12);
    controls.setFillColor(sf::Color(50, 55, 65, 120));
    sf::FloatRect ctrlBounds = controls.getLocalBounds();
    controls.setOrigin(ctrlBounds.left + ctrlBounds.width / 2.0f,
                       ctrlBounds.top + ctrlBounds.height / 2.0f);
    controls.setPosition(winW / 2.0f, winH * 0.92f);
    window.draw(controls);

    // Atmospheric particles even on menu
    particleSystem->render(window);
}

void Game::renderPause() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

    // Dim overlay
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(0, 0, 0, 150));
    window.draw(overlay);

    // "PAUSED" title
    sf::Text pauseTitle;
    pauseTitle.setFont(font);
    pauseTitle.setString("PAUSED");
    pauseTitle.setCharacterSize(36);
    pauseTitle.setFillColor(sf::Color(160, 170, 190));
    sf::FloatRect pauseBounds = pauseTitle.getLocalBounds();
    pauseTitle.setOrigin(pauseBounds.left + pauseBounds.width / 2.0f,
                         pauseBounds.top + pauseBounds.height / 2.0f);
    pauseTitle.setPosition(winW / 2.0f, winH / 2.0f - 30.0f);
    window.draw(pauseTitle);

    // Controls hint
    sf::Text hint;
    hint.setFont(font);
    hint.setString("ESC  Resume  |  Q  Quit to Menu");
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(80, 90, 110, 180));
    sf::FloatRect hintBounds = hint.getLocalBounds();
    hint.setOrigin(hintBounds.left + hintBounds.width / 2.0f,
                   hintBounds.top + hintBounds.height / 2.0f);
    hint.setPosition(winW / 2.0f, winH / 2.0f + 20.0f);
    window.draw(hint);
}

void Game::renderEnding() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);
    sf::Font& font = fonts.get("main", "assets/fonts/main.ttf");

    // Phase 1: White fade-in (0-5 seconds)
    if (endingTimer < 5.0f) {
        // Show the game world fading to white
        if (tileMap) tileMap->render(window, physics);
        for (auto it = entities.begin(); it != entities.end(); ++it) {
            (*it)->render(window);
        }

        sf::RectangleShape whiteFade;
        whiteFade.setSize(sf::Vector2f(winW, winH));
        int alpha = static_cast<int>((endingTimer / 5.0f) * 255.0f);
        whiteFade.setFillColor(sf::Color(255, 255, 255, alpha));
        window.draw(whiteFade);
    }
    // Phase 2: White screen with wake-up text (5-7 seconds)
    else if (endingTimer < 7.0f) {
        window.clear(sf::Color::White);

        float t = (endingTimer - 5.5f) / 1.5f;
        if (t > 0.0f) {
            int textAlpha = static_cast<int>(std::min(t, 1.0f) * 180.0f);
            sf::Text wakeUp;
            wakeUp.setFont(font);
            wakeUp.setString("...");
            wakeUp.setCharacterSize(28);
            wakeUp.setFillColor(sf::Color(60, 60, 60, textAlpha));
            sf::FloatRect wakeBounds = wakeUp.getLocalBounds();
            wakeUp.setOrigin(wakeBounds.left + wakeBounds.width / 2.0f,
                             wakeBounds.top + wakeBounds.height / 2.0f);
            wakeUp.setPosition(winW / 2.0f, winH / 2.0f);
            window.draw(wakeUp);
        }
    }
    // Phase 3: Fade to desk scene (7-12 seconds)
    else if (endingTimer < 12.0f) {
        float t = (endingTimer - 7.0f) / 5.0f;

        // Desk scene background color (warm, lit room)
        int r = static_cast<int>(255.0f - t * 215.0f);  // 255 -> 40
        int g = static_cast<int>(255.0f - t * 220.0f);  // 255 -> 35
        int b = static_cast<int>(255.0f - t * 225.0f);  // 255 -> 30
        window.clear(sf::Color(r, g, b));

        // Simple desk representation
        if (t > 0.3f) {
            float deskAlpha = std::min((t - 0.3f) / 0.5f, 1.0f) * 255.0f;

            // Desk surface
            sf::RectangleShape desk;
            desk.setSize(sf::Vector2f(winW * 0.6f, 8.0f));
            desk.setOrigin(winW * 0.3f, 4.0f);
            desk.setPosition(winW / 2.0f, winH * 0.65f);
            desk.setFillColor(sf::Color(60, 40, 25, static_cast<int>(deskAlpha)));
            window.draw(desk);

            // Monitor
            sf::RectangleShape monitor;
            monitor.setSize(sf::Vector2f(200.0f, 140.0f));
            monitor.setOrigin(100.0f, 70.0f);
            monitor.setPosition(winW / 2.0f, winH * 0.45f);
            monitor.setFillColor(sf::Color(20, 30, 40, static_cast<int>(deskAlpha)));
            monitor.setOutlineColor(sf::Color(50, 50, 55, static_cast<int>(deskAlpha)));
            monitor.setOutlineThickness(2.0f);
            window.draw(monitor);

            // Text on monitor screen -- deadline due
            if (t > 0.6f) {
                float monTextAlpha = std::min((t - 0.6f) / 0.3f, 1.0f) * deskAlpha;
                sf::Text monitorText;
                monitorText.setFont(font);
                monitorText.setString("DEADLINE: 11:59 PM");
                monitorText.setCharacterSize(11);
                monitorText.setFillColor(sf::Color(100, 160, 120, static_cast<int>(monTextAlpha)));
                sf::FloatRect monBounds = monitorText.getLocalBounds();
                monitorText.setOrigin(monBounds.left + monBounds.width / 2.0f,
                                      monBounds.top + monBounds.height / 2.0f);
                monitorText.setPosition(winW / 2.0f, winH * 0.43f);
                window.draw(monitorText);
            }

            // Keyboard
            sf::RectangleShape keyboard;
            keyboard.setSize(sf::Vector2f(160.0f, 20.0f));
            keyboard.setOrigin(80.0f, 10.0f);
            keyboard.setPosition(winW / 2.0f, winH * 0.68f);
            keyboard.setFillColor(sf::Color(35, 35, 38, static_cast<int>(deskAlpha)));
            window.draw(keyboard);
        }
    }
    // Phase 4: Ethan wakes up -- hold scene, then fade to black
    else {
        window.clear(sf::Color(40, 35, 30));

        // Desk scene (fully visible)
        float winHalf = winW / 2.0f;

        sf::RectangleShape desk;
        desk.setSize(sf::Vector2f(winW * 0.6f, 8.0f));
        desk.setOrigin(winW * 0.3f, 4.0f);
        desk.setPosition(winHalf, winH * 0.65f);
        desk.setFillColor(sf::Color(60, 40, 25));
        window.draw(desk);

        sf::RectangleShape monitor;
        monitor.setSize(sf::Vector2f(200.0f, 140.0f));
        monitor.setOrigin(100.0f, 70.0f);
        monitor.setPosition(winHalf, winH * 0.45f);
        monitor.setFillColor(sf::Color(20, 30, 40));
        monitor.setOutlineColor(sf::Color(50, 50, 55));
        monitor.setOutlineThickness(2.0f);
        window.draw(monitor);

        // Deadline text on monitor
        sf::Text monitorText;
        monitorText.setFont(font);
        monitorText.setString("DEADLINE: 11:59 PM");
        monitorText.setCharacterSize(11);
        monitorText.setFillColor(sf::Color(100, 160, 120));
        sf::FloatRect monBounds = monitorText.getLocalBounds();
        monitorText.setOrigin(monBounds.left + monBounds.width / 2.0f,
                              monBounds.top + monBounds.height / 2.0f);
        monitorText.setPosition(winHalf, winH * 0.43f);
        window.draw(monitorText);

        sf::RectangleShape keyboard;
        keyboard.setSize(sf::Vector2f(160.0f, 20.0f));
        keyboard.setOrigin(80.0f, 10.0f);
        keyboard.setPosition(winHalf, winH * 0.68f);
        keyboard.setFillColor(sf::Color(35, 35, 38));
        window.draw(keyboard);

        // Narrative text below desk -- appears after a beat
        if (endingTimer > 13.0f && endingTimer < 16.0f) {
            float narrativeAlpha = 0.0f;
            if (endingTimer < 14.0f)
                narrativeAlpha = (endingTimer - 13.0f) * 255.0f;
            else if (endingTimer > 15.0f)
                narrativeAlpha = (16.0f - endingTimer) * 255.0f;
            else
                narrativeAlpha = 255.0f;

            sf::Text narrative;
            narrative.setFont(font);
            narrative.setString("It was just a dream... wasn't it?");
            narrative.setCharacterSize(18);
            narrative.setFillColor(sf::Color(150, 140, 130, static_cast<int>(narrativeAlpha)));
            sf::FloatRect narBounds = narrative.getLocalBounds();
            narrative.setOrigin(narBounds.left + narBounds.width / 2.0f,
                                narBounds.top + narBounds.height / 2.0f);
            narrative.setPosition(winHalf, winH * 0.82f);
            window.draw(narrative);
        }

        // Fade to black after 15 seconds
        if (endingTimer > 15.0f) {
            float fadeT = std::min((endingTimer - 15.0f) / 3.0f, 1.0f);
            sf::RectangleShape blackFade;
            blackFade.setSize(sf::Vector2f(winW, winH));
            blackFade.setFillColor(sf::Color(0, 0, 0, static_cast<int>(fadeT * 255.0f)));
            window.draw(blackFade);

            // Final title card during fade
            if (endingTimer > 16.5f && endingTimer < 19.5f) {
                float titleAlpha = 0.0f;
                if (endingTimer < 17.5f)
                    titleAlpha = (endingTimer - 16.5f) * 255.0f;
                else if (endingTimer > 18.5f)
                    titleAlpha = (19.5f - endingTimer) * 255.0f;
                else
                    titleAlpha = 255.0f;

                sf::Text finalTitle;
                finalTitle.setFont(font);
                finalTitle.setString("DEADLINE'S  EDGE");
                finalTitle.setCharacterSize(42);
                finalTitle.setFillColor(sf::Color(150, 160, 180, static_cast<int>(titleAlpha)));
                sf::FloatRect ftBounds = finalTitle.getLocalBounds();
                finalTitle.setOrigin(ftBounds.left + ftBounds.width / 2.0f,
                                     ftBounds.top + ftBounds.height / 2.0f);
                finalTitle.setPosition(winHalf, winH / 2.0f);
                window.draw(finalTitle);
            }

            // After full fade, close
            if (endingTimer > 20.0f) {
                window.close();
            }
        }
    }
}

void Game::renderVignette() {
    window.draw(vignetteTop);
    window.draw(vignetteBottom);
    window.draw(vignetteLeft);
    window.draw(vignetteRight);

    // Proximity distortion near Remnants
    if (player) {
        float closestDist = 999.0f;
        for (auto it = remnants.begin(); it != remnants.end(); ++it) {
            float d = (*it)->getDistanceToPlayer();
            if (d < closestDist) closestDist = d;
        }

        // Screen distortion overlay when very close to a Remnant
        if (closestDist < Remnant::DETECTION_RANGE) {
            float intensity = 1.0f - (closestDist / Remnant::DETECTION_RANGE);
            int alpha = static_cast<int>(intensity * 40.0f);

            float winW = static_cast<float>(window.getSize().x);
            float winH = static_cast<float>(window.getSize().y);

            sf::RectangleShape distortion;
            distortion.setSize(sf::Vector2f(winW, winH));
            // Slight red-shift when near Remnants
            distortion.setFillColor(sf::Color(30, 5, 5, alpha));
            window.draw(distortion);
        }
    }
}
