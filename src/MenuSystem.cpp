#include "MenuSystem.h"
#include "AssetManager.h"
#include "AssetKeys.h"
#include "InputManager.h"
#include "SaveSystem.h"
#include <cmath>

MenuSystem::MenuSystem(sf::RenderWindow& window, AssetManager* assets)
    : windowRef(window)
    , assets(assets)
    , mainMenuSelection(MenuSelection::Play)
    , settingsSection(SettingsSection::Audio)
    , selectedIndex(0)
    , maxMenuItems(5)
    , pauseSelection(0)
    , settingsSelection(0)
    , gameOverSelection(0)
    , mainSelection(0)
    , shopSelection(0)
    , shopMaxItems(4)
    , masterVolume(1.0f)
    , musicVolume(0.7f)
    , sfxVolume(0.8f)
    , fullscreen(true)
    , pulseTimer(0.0f)
    , bgScrollOffset(0.0f)
    , titleBounce(0.0f)
    , goScore(0.0f)
    , goBestScore(0.0f)
    , goCoins(0)
    , goObstaclesDodged(0)
    , goMaxCombo(0)
    , goDistance(0.0f)
{
}

void MenuSystem::handleInput(const InputManager& input, GameState& currentState) {
    // Input is now handled in processEvents in Game.cpp
    // This method left for additional processing if needed
}

void MenuSystem::update(float dt) {
    pulseTimer += dt;
    bgScrollOffset += dt * 30.0f;
    titleBounce = std::sin(pulseTimer * 2.0f) * 5.0f;
}

void MenuSystem::navigateUp() {
    selectedIndex--;
    if (selectedIndex < 0) selectedIndex = maxMenuItems - 1;

    pauseSelection      = selectedIndex;
    settingsSelection   = selectedIndex;
    gameOverSelection   = selectedIndex;
    mainSelection       = selectedIndex;
    shopSelection       = selectedIndex % shopMaxItems;
}

void MenuSystem::navigateDown() {
    selectedIndex++;
    if (selectedIndex >= maxMenuItems) selectedIndex = 0;

    pauseSelection      = selectedIndex;
    settingsSelection   = selectedIndex;
    gameOverSelection   = selectedIndex;
    mainSelection       = selectedIndex;
    shopSelection       = selectedIndex % shopMaxItems;
}

void MenuSystem::navigateLeft() {
    // For sliders in settings
    float delta = -0.1f;
    switch (settingsSelection) {
        case 0: masterVolume = std::max(0.0f, std::min(1.0f, masterVolume + delta)); break;
        case 1: musicVolume = std::max(0.0f, std::min(1.0f, musicVolume + delta)); break;
        case 2: sfxVolume = std::max(0.0f, std::min(1.0f, sfxVolume + delta)); break;
    }
}

void MenuSystem::navigateRight() {
    // For sliders in settings
    float delta = 0.1f;
    switch (settingsSelection) {
        case 0: masterVolume = std::max(0.0f, std::min(1.0f, masterVolume + delta)); break;
        case 1: musicVolume = std::max(0.0f, std::min(1.0f, musicVolume + delta)); break;
        case 2: sfxVolume = std::max(0.0f, std::min(1.0f, sfxVolume + delta)); break;
    }
}

void MenuSystem::select(GameState& currentState, SaveSystem* saveSystem) {
    switch (currentState) {
        case GameState::Menu:
            maxMenuItems = 5;
            switch (mainSelection) {
                case 0: currentState = GameState::Play; break;
                case 1: currentState = GameState::Settings; settingsSelection = 0; selectedIndex = 0; maxMenuItems = 5; break;
                case 2: currentState = GameState::Shop; selectedIndex = 0; shopSelection = 0; maxMenuItems = shopMaxItems; break;
                case 3: currentState = GameState::Achievements; selectedIndex = 0; break;
                case 4: windowRef.close(); break;
            }
            break;

        case GameState::Settings:
            if (settingsSelection == 4) { // Back
                currentState = GameState::Menu;
                selectedIndex = 1; // Return to settings option
                mainSelection = 1;
            }
            break;

        case GameState::Shop:
            if (shopSelection == shopMaxItems - 1) {
                // Back button selected
                currentState = GameState::Menu;
                selectedIndex = 0;
                mainSelection = 0;
                maxMenuItems = 5;
            } else if (saveSystem) {
                // Purchase logic
                PlayerData& pd = saveSystem->getData();
                switch (shopSelection) {
                    case 0: // Shield Start — 75 coins, one-time
                        if (!pd.shieldStartUpgrade && saveSystem->spendCoins(75)) {
                            pd.shieldStartUpgrade = true;
                            saveSystem->save();
                        }
                        break;
                    case 1: // Double Coins — 100 coins, one-time
                        if (!pd.doubleCoinsUpgrade && saveSystem->spendCoins(100)) {
                            pd.doubleCoinsUpgrade = true;
                            saveSystem->save();
                        }
                        break;
                    case 2: // Extra Life — 150 coins each, up to 3
                        if (pd.extraLives < 3 && saveSystem->spendCoins(150)) {
                            pd.extraLives++;
                            saveSystem->save();
                        }
                        break;
                }
            }
            break;

        case GameState::Achievements:
            // Escape/back handled elsewhere
            break;

        default: break;
    }
}

void MenuSystem::back(GameState& currentState) {
    switch (currentState) {
        case GameState::Settings:
        case GameState::Shop:
        case GameState::Achievements:
            currentState = GameState::Menu;
            selectedIndex = 0;
            mainSelection = 0;
            maxMenuItems = 5;
            break;
        default:
            break;
    }
}

MenuSelection MenuSystem::getPauseSelection() const {
    switch (pauseSelection) {
        case 0: return MenuSelection::Resume;
        case 1: return MenuSelection::Settings;
        case 2: return MenuSelection::QuitToMenu;
        default: return MenuSelection::Resume;
    }
}

MenuSelection MenuSystem::getGameOverSelection() const {
    switch (gameOverSelection) {
        case 0: return MenuSelection::Retry;
        case 1: return MenuSelection::QuitToMenu;
        default: return MenuSelection::Retry;
    }
}

void MenuSystem::resetMenuState() {
    selectedIndex = 0;
    mainMenuSelection = MenuSelection::Play;
    pauseSelection = 0;
    settingsSelection = 0;
    gameOverSelection = 0;
    mainSelection = 0;
    maxMenuItems = 5;
}

void MenuSystem::resetPauseSelection() {
    pauseSelection = 0;
    selectedIndex = 0;
    maxMenuItems = 3;
}

void MenuSystem::setGameOverStats(float score, float bestScore, int coins, int obstaclesDodged,
                                   int maxCombo, float distance) {
    goScore = score;
    goBestScore = bestScore;
    goCoins = coins;
    goObstaclesDodged = obstaclesDodged;
    goMaxCombo = maxCombo;
    goDistance = distance;
    gameOverSelection = 0;
    selectedIndex = 0;
    maxMenuItems = 2;
}

void MenuSystem::render(sf::RenderWindow& window, sf::Font& font, GameState state, SaveSystem* saveSystem) {
    switch (state) {
        case GameState::Menu: renderMainMenu(window, font); break;
        case GameState::Settings: renderSettingsMenu(window, font); break;
        case GameState::Pause: renderPauseMenu(window, font); break;
        case GameState::GameOver: renderGameOverMenu(window, font); break;
        case GameState::Shop:
            renderShopMenu(window, font, saveSystem);
            break;
        case GameState::Achievements: renderAchievementsMenu(window, font); break;
        default: break;
    }
}

void MenuSystem::renderMainMenu(sf::RenderWindow& window, sf::Font& font) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    maxMenuItems = 5;

    // Semi-transparent background panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(500.0f, 420.0f));
    panel.setFillColor(sf::Color(15, 18, 28, 230));
    panel.setOutlineColor(Constants::UI_PRIMARY);
    panel.setOutlineThickness(2.0f);
    panel.setOrigin(250.0f, 210.0f);
    panel.setPosition(winW * 0.5f, winH * 0.5f);
    window.draw(panel);

    // Title with bounce
    drawTitle(window, font, "DEADLINE'S EDGE", winH * 0.28f + titleBounce);

    // Subtitle
    sf::Text sub;
    sub.setFont(font);
    sub.setString("Campus Sprint");
    sub.setCharacterSize(20);
    sub.setFillColor(Constants::UI_TEXT_DIM);
    sf::FloatRect subBounds = sub.getLocalBounds();
    sub.setOrigin(subBounds.left + subBounds.width * 0.5f, subBounds.top + subBounds.height * 0.5f);
    sub.setPosition(winW * 0.5f, winH * 0.36f);
    window.draw(sub);

    // Menu items
    float itemY = winH * 0.45f;
    float itemSpacing = 50.0f;

    drawMenuItem(window, font, "PLAY", itemY, mainSelection == 0);
    drawMenuItem(window, font, "SETTINGS", itemY + itemSpacing, mainSelection == 1);
    drawMenuItem(window, font, "SHOP", itemY + itemSpacing * 2, mainSelection == 2);
    drawMenuItem(window, font, "ACHIEVEMENTS", itemY + itemSpacing * 3, mainSelection == 3);
    drawMenuItem(window, font, "QUIT", itemY + itemSpacing * 4, mainSelection == 4);

    // Controls hint
    float pulse = (std::sin(pulseTimer * 3.0f) + 1.0f) * 0.5f;
    sf::Text hint;
    hint.setFont(font);
    hint.setString("Use ARROWS to navigate, ENTER to select");
    hint.setCharacterSize(14);
    hint.setFillColor(sf::Color(130, 142, 164, static_cast<sf::Uint8>(120 + pulse * 80)));
    sf::FloatRect hintBounds = hint.getLocalBounds();
    hint.setOrigin(hintBounds.left + hintBounds.width * 0.5f, hintBounds.top + hintBounds.height * 0.5f);
    hint.setPosition(winW * 0.5f, winH * 0.88f);
    window.draw(hint);
}

void MenuSystem::renderSettingsMenu(sf::RenderWindow& window, sf::Font& font) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    maxMenuItems = 5;

    // Background overlay
    drawDarkOverlay(window, 200);

    // Panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(600.0f, 420.0f));
    panel.setFillColor(sf::Color(20, 24, 35, 245));
    panel.setOutlineColor(Constants::UI_PRIMARY);
    panel.setOutlineThickness(2.0f);
    panel.setOrigin(300.0f, 210.0f);
    panel.setPosition(winW * 0.5f, winH * 0.5f);
    window.draw(panel);

    drawTitle(window, font, "SETTINGS", winH * 0.32f);

    float itemY = winH * 0.42f;
    float spacing = 50.0f;

    drawSlider(window, font, "Master Volume", masterVolume, itemY, settingsSelection == 0);
    drawSlider(window, font, "Music Volume", musicVolume, itemY + spacing, settingsSelection == 1);
    drawSlider(window, font, "SFX Volume", sfxVolume, itemY + spacing * 2, settingsSelection == 2);
    drawMenuItem(window, font, fullscreen ? "FULLSCREEN: ON" : "FULLSCREEN: OFF", itemY + spacing * 3, settingsSelection == 3);
    drawMenuItem(window, font, "< BACK", itemY + spacing * 4 + 10.0f, settingsSelection == 4);

    // Controls hint
    sf::Text hint;
    hint.setFont(font);
    hint.setString("LEFT/RIGHT to adjust, ENTER to toggle/back");
    hint.setCharacterSize(14);
    hint.setFillColor(Constants::UI_TEXT_DIM);
    sf::FloatRect hintBounds = hint.getLocalBounds();
    hint.setOrigin(hintBounds.left + hintBounds.width * 0.5f, hintBounds.top + hintBounds.height * 0.5f);
    hint.setPosition(winW * 0.5f, winH * 0.82f);
    window.draw(hint);
}

void MenuSystem::renderPauseMenu(sf::RenderWindow& window, sf::Font& font) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    maxMenuItems = 3;

    // Dark overlay
    drawDarkOverlay(window, 180);

    // Panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(400.0f, 300.0f));
    panel.setFillColor(sf::Color(20, 24, 35, 240));
    panel.setOutlineColor(Constants::UI_PRIMARY);
    panel.setOutlineThickness(2.0f);
    panel.setOrigin(200.0f, 150.0f);
    panel.setPosition(winW * 0.5f, winH * 0.5f);
    window.draw(panel);

    drawTitle(window, font, "PAUSED", winH * 0.38f);

    float itemY = winH * 0.48f;
    float spacing = 50.0f;

    drawMenuItem(window, font, "RESUME", itemY, pauseSelection == 0);
    drawMenuItem(window, font, "SETTINGS", itemY + spacing, pauseSelection == 1);
    drawMenuItem(window, font, "MAIN MENU", itemY + spacing * 2, pauseSelection == 2);
}

void MenuSystem::renderGameOverMenu(sf::RenderWindow& window, sf::Font& font) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    maxMenuItems = 2;

    // Red-tinted overlay
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(30, 0, 0, 190));
    window.draw(overlay);

    // Panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(520.0f, 480.0f));
    panel.setFillColor(sf::Color(20, 15, 18, 245));
    panel.setOutlineColor(Constants::UI_DANGER);
    panel.setOutlineThickness(2.0f);
    panel.setOrigin(260.0f, 240.0f);
    panel.setPosition(winW * 0.5f, winH * 0.5f);
    window.draw(panel);

    // Title
    sf::Text title;
    title.setFont(font);
    title.setString("DEADLINE MISSED!");
    title.setCharacterSize(42);
    title.setFillColor(Constants::UI_DANGER);
    title.setStyle(sf::Text::Bold);
    sf::FloatRect titleBounds = title.getLocalBounds();
    title.setOrigin(titleBounds.left + titleBounds.width * 0.5f, titleBounds.top + titleBounds.height * 0.5f);
    title.setPosition(winW * 0.5f, winH * 0.30f);
    window.draw(title);

    // Stats
    float statsY = winH * 0.40f;
    float statsSpacing = 32.0f;

    // Score
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString("Score: " + std::to_string(static_cast<int>(goScore)));
    scoreText.setCharacterSize(28);
    scoreText.setFillColor(Constants::UI_TEXT_LIGHT);
    sf::FloatRect scoreBounds = scoreText.getLocalBounds();
    scoreText.setOrigin(scoreBounds.left + scoreBounds.width * 0.5f, scoreBounds.top + scoreBounds.height * 0.5f);
    scoreText.setPosition(winW * 0.5f, statsY);
    window.draw(scoreText);

    // Best score
    bool newBest = (goScore >= goBestScore && goScore > 0);
    sf::Text bestText;
    bestText.setFont(font);
    bestText.setString(newBest ? "NEW BEST!" : ("Best: " + std::to_string(static_cast<int>(goBestScore))));
    bestText.setCharacterSize(20);
    bestText.setFillColor(newBest ? Constants::UI_SECONDARY : Constants::UI_TEXT_DIM);
    if (newBest) bestText.setStyle(sf::Text::Bold);
    sf::FloatRect bestBounds = bestText.getLocalBounds();
    bestText.setOrigin(bestBounds.left + bestBounds.width * 0.5f, bestBounds.top + bestBounds.height * 0.5f);
    bestText.setPosition(winW * 0.5f, statsY + statsSpacing);
    window.draw(bestText);

    // Distance
    sf::Text distText;
    distText.setFont(font);
    distText.setString("Distance: " + std::to_string(static_cast<int>(goDistance)) + "m");
    distText.setCharacterSize(18);
    distText.setFillColor(Constants::UI_TEXT_DIM);
    sf::FloatRect distBounds = distText.getLocalBounds();
    distText.setOrigin(distBounds.left + distBounds.width * 0.5f, distBounds.top + distBounds.height * 0.5f);
    distText.setPosition(winW * 0.5f, statsY + statsSpacing * 2);
    window.draw(distText);

    // Coins earned
    sf::Text coinsText;
    coinsText.setFont(font);
    coinsText.setString("Coins: +" + std::to_string(goCoins));
    coinsText.setCharacterSize(20);
    coinsText.setFillColor(Constants::UI_SECONDARY);
    sf::FloatRect coinsBounds = coinsText.getLocalBounds();
    coinsText.setOrigin(coinsBounds.left + coinsBounds.width * 0.5f, coinsBounds.top + coinsBounds.height * 0.5f);
    coinsText.setPosition(winW * 0.5f, statsY + statsSpacing * 3);
    window.draw(coinsText);

    // Max combo
    sf::Text comboText;
    comboText.setFont(font);
    comboText.setString("Max Combo: " + std::to_string(goMaxCombo) + "x");
    comboText.setCharacterSize(18);
    comboText.setFillColor(Constants::UI_ACCENT);
    sf::FloatRect comboBounds = comboText.getLocalBounds();
    comboText.setOrigin(comboBounds.left + comboBounds.width * 0.5f, comboBounds.top + comboBounds.height * 0.5f);
    comboText.setPosition(winW * 0.5f, statsY + statsSpacing * 4);
    window.draw(comboText);

    // Obstacles dodged
    sf::Text obstaclesText;
    obstaclesText.setFont(font);
    obstaclesText.setString("Obstacles Dodged: " + std::to_string(goObstaclesDodged));
    obstaclesText.setCharacterSize(18);
    obstaclesText.setFillColor(Constants::UI_TEXT_DIM);
    sf::FloatRect obstaclesBounds = obstaclesText.getLocalBounds();
    obstaclesText.setOrigin(obstaclesBounds.left + obstaclesBounds.width * 0.5f, obstaclesBounds.top + obstaclesBounds.height * 0.5f);
    obstaclesText.setPosition(winW * 0.5f, statsY + statsSpacing * 5);
    window.draw(obstaclesText);

    // Menu options
    float itemY = winH * 0.70f;
    drawMenuItem(window, font, "RETRY", itemY, gameOverSelection == 0);
    drawMenuItem(window, font, "MAIN MENU", itemY + 45.0f, gameOverSelection == 1);
}

void MenuSystem::renderShopMenu(sf::RenderWindow& window, sf::Font& font, SaveSystem* saveSystem) {
    int playerCoins = saveSystem ? saveSystem->getData().totalCoins : 0;
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    shopMaxItems = 4;   // 3 items + back
    maxMenuItems = shopMaxItems;

    drawDarkOverlay(window, 200);

    // Panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(760.0f, 540.0f));
    panel.setFillColor(sf::Color(14, 18, 28, 248));
    panel.setOutlineColor(Constants::UI_SECONDARY);
    panel.setOutlineThickness(2.0f);
    panel.setOrigin(380.0f, 270.0f);
    panel.setPosition(winW * 0.5f, winH * 0.5f);
    window.draw(panel);

    drawTitle(window, font, "UPGRADE SHOP", winH * 0.26f + titleBounce * 0.5f);

    // Coin display
    drawCenteredText(window, font, "Coins: " + std::to_string(playerCoins),
                     winH * 0.345f, 26, Constants::UI_SECONDARY);

    // Separator line
    sf::RectangleShape sep(sf::Vector2f(660.0f, 1.0f));
    sep.setFillColor(sf::Color(255, 193, 7, 80));
    sep.setOrigin(330.0f, 0.0f);
    sep.setPosition(winW * 0.5f, winH * 0.385f);
    window.draw(sep);

    // ---- Shop items ----
    // We need the SaveSystem data — passed via playerCoins only, so we render based on
    // the selection index and let the caller (select()) do the actual logic.
    // We show affordability via the playerCoins value passed in.

    struct ShopEntry { std::string name; std::string desc; int cost; bool owned; int ownedCount; int maxCount; };
    // Note: we can't access SaveSystem here directly, so we use stored flags from last render.
    // Instead, render with data passed through playerCoins — we'll add a render overload.
    // For now render generic affordable state based on playerCoins only.

    // We'll use a helper lambda drawing a full item card
    auto drawItemCard = [&](int idx, const std::string& name, const std::string& desc,
                             int cost, bool owned, int ownedCount, int maxCount) {
        float cardY = winH * 0.43f + idx * 90.0f;
        bool sel = (shopSelection == idx);
        bool affordable = (playerCoins >= cost);
        bool canBuy = !owned && affordable;
        if (maxCount > 1) canBuy = (ownedCount < maxCount) && affordable;

        // Card background
        sf::RectangleShape card(sf::Vector2f(680.0f, 75.0f));
        card.setOrigin(340.0f, 37.5f);
        card.setPosition(winW * 0.5f, cardY);
        sf::Color cardColor = sel ? sf::Color(30, 40, 65, 220) : sf::Color(20, 26, 40, 180);
        card.setFillColor(cardColor);
        if (sel) {
            card.setOutlineColor(Constants::UI_PRIMARY);
            card.setOutlineThickness(1.5f);
        }
        window.draw(card);

        // Name
        sf::Text nameT;
        nameT.setFont(font);
        nameT.setString(name);
        nameT.setCharacterSize(20);
        nameT.setFillColor(sel ? Constants::UI_TEXT_LIGHT : Constants::UI_TEXT_DIM);
        if (sel) nameT.setStyle(sf::Text::Bold);
        nameT.setPosition(winW * 0.5f - 320.0f, cardY - 28.0f);
        window.draw(nameT);

        // Description
        sf::Text descT;
        descT.setFont(font);
        descT.setString(desc);
        descT.setCharacterSize(15);
        descT.setFillColor(sf::Color(140, 155, 175));
        descT.setPosition(winW * 0.5f - 320.0f, cardY + 2.0f);
        window.draw(descT);

        // Right side: price or status
        std::string rightLabel;
        sf::Color rightColor;
        if (owned && maxCount == 1) {
            rightLabel = "OWNED";
            rightColor = Constants::UI_ACCENT;
        } else if (maxCount > 1) {
            rightLabel = std::to_string(ownedCount) + "/" + std::to_string(maxCount);
            rightColor = (ownedCount >= maxCount) ? Constants::UI_ACCENT : Constants::UI_SECONDARY;
            if (ownedCount < maxCount) rightLabel += "  (" + std::to_string(cost) + "c)";
        } else {
            rightLabel = std::to_string(cost) + " coins";
            rightColor = affordable ? Constants::UI_SECONDARY : Constants::UI_DANGER;
        }

        sf::Text priceT;
        priceT.setFont(font);
        priceT.setString(rightLabel);
        priceT.setCharacterSize(18);
        priceT.setFillColor(rightColor);
        sf::FloatRect pb = priceT.getLocalBounds();
        priceT.setOrigin(pb.left + pb.width, pb.top + pb.height * 0.5f);
        priceT.setPosition(winW * 0.5f + 325.0f, cardY - 10.0f);
        window.draw(priceT);

        // "PRESS ENTER" hint on selected purchasable item
        if (sel && canBuy) {
            float pulse = (std::sin(pulseTimer * 5.0f) + 1.0f) * 0.5f;
            sf::Uint8 a = static_cast<sf::Uint8>(100 + pulse * 155);
            sf::Text hint;
            hint.setFont(font);
            hint.setString("ENTER to buy");
            hint.setCharacterSize(13);
            hint.setFillColor(sf::Color(255, 193, 7, a));
            sf::FloatRect hb = hint.getLocalBounds();
            hint.setOrigin(hb.left + hb.width, hb.top + hb.height * 0.5f);
            hint.setPosition(winW * 0.5f + 325.0f, cardY + 20.0f);
            window.draw(hint);
        }

        // Selection arrow
        if (sel) {
            sf::Text arrow;
            arrow.setFont(font);
            arrow.setString(">");
            arrow.setCharacterSize(22);
            float pulse = (std::sin(pulseTimer * 4.0f) + 1.0f) * 0.5f;
            arrow.setFillColor(sf::Color(41, 98, 255, static_cast<sf::Uint8>(180 + pulse * 75)));
            arrow.setPosition(winW * 0.5f - 345.0f, cardY - 14.0f);
            window.draw(arrow);
        }
    };

    // Retrieve owned state from last-known save (playerCoins is our only channel in).
    // We track purchase state via the IDs baked into select(). For display we need
    // to pass the SaveSystem — but renderShopMenu only gets playerCoins.
    // Workaround: store owned flags as members updated each frame.
    // We already have access via the assets pointer chain — instead just display based
    // on stored shop state passed through a new render call in render().
    // SIMPLEST: We added saveSystem pointer to MenuSystem ctor — but it's not there.
    // So we promote to using the SaveSystem* that comes into render():

    bool shieldOwned = saveSystem && saveSystem->getData().shieldStartUpgrade;
    bool dblOwned    = saveSystem && saveSystem->getData().doubleCoinsUpgrade;
    int  livesOwned  = saveSystem ? saveSystem->getData().extraLives : 0;

    drawItemCard(0, "Shield Start", "Begin each run with an active shield",       75,  shieldOwned, 0,          1);
    drawItemCard(1, "Double Coins", "Earn 2x coins during every run",             100, dblOwned,    0,          1);
    drawItemCard(2, "Extra Life",   "Start with an extra hit  (max 3 stackable)", 150, false,       livesOwned, 3);

    drawMenuItem(window, font, "< BACK  (ESC)", winH * 0.855f, shopSelection == 3);
}

void MenuSystem::renderAchievementsMenu(sf::RenderWindow& window, sf::Font& font) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Background
    drawDarkOverlay(window, 200);

    // Panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(700.0f, 500.0f));
    panel.setFillColor(sf::Color(20, 24, 35, 245));
    panel.setOutlineColor(Constants::UI_ACCENT);
    panel.setOutlineThickness(2.0f);
    panel.setOrigin(350.0f, 250.0f);
    panel.setPosition(winW * 0.5f, winH * 0.5f);
    window.draw(panel);

    drawTitle(window, font, "ACHIEVEMENTS", winH * 0.30f);

    // Placeholder achievements
    float itemY = winH * 0.42f;
    drawMenuItem(window, font, "[X] First Run - Complete your first run", itemY, false);
    drawMenuItem(window, font, "[ ] Score 500 - Reach 500 points", itemY + 40.0f, false);
    drawMenuItem(window, font, "[ ] Combo Master - Get a 10x combo", itemY + 80.0f, false);
    drawMenuItem(window, font, "[ ] Collector - Collect 100 coins total", itemY + 120.0f, false);

    drawMenuItem(window, font, "< BACK (ESC)", winH * 0.72f, true);
}

void MenuSystem::drawDarkOverlay(sf::RenderWindow& window, sf::Uint8 alpha) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(winW, winH));
    overlay.setFillColor(sf::Color(0, 0, 0, alpha));
    window.draw(overlay);
}

void MenuSystem::drawTitle(sf::RenderWindow& window, sf::Font& font, const std::string& title, float y) {
    float winW = static_cast<float>(window.getSize().x);

    sf::Text text;
    text.setFont(font);
    text.setString(title);
    text.setCharacterSize(48);
    text.setFillColor(Constants::UI_TEXT_LIGHT);
    text.setStyle(sf::Text::Bold);
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    text.setPosition(winW * 0.5f, y);
    window.draw(text);
}

void MenuSystem::drawMenuItem(sf::RenderWindow& window, sf::Font& font, const std::string& text, float y, bool selected, bool enabled) {
    float winW = static_cast<float>(window.getSize().x);

    sf::Text item;
    item.setFont(font);
    item.setString(text);
    item.setCharacterSize(Constants::HUD_FONT_SIZE_MEDIUM);

    if (!enabled) {
        item.setFillColor(sf::Color(80, 85, 95));
    } else if (selected) {
        float pulse = (std::sin(pulseTimer * 4.0f) + 1.0f) * 0.5f;
        sf::Uint8 alpha = static_cast<sf::Uint8>(200 + pulse * 55);
        item.setFillColor(sf::Color(Constants::UI_PRIMARY.r, Constants::UI_PRIMARY.g, Constants::UI_PRIMARY.b, alpha));
        item.setStyle(sf::Text::Bold);

        // Selection indicator
        sf::Text arrow;
        arrow.setFont(font);
        arrow.setString(">");
        arrow.setCharacterSize(Constants::HUD_FONT_SIZE_MEDIUM);
        arrow.setFillColor(Constants::UI_PRIMARY);
        sf::FloatRect itemBounds = item.getLocalBounds();
        arrow.setPosition(winW * 0.5f - itemBounds.width * 0.5f - 30.0f, y - itemBounds.height * 0.5f - 4.0f);
        window.draw(arrow);
    } else {
        item.setFillColor(Constants::UI_TEXT_DIM);
    }

    sf::FloatRect bounds = item.getLocalBounds();
    item.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    item.setPosition(winW * 0.5f, y);
    window.draw(item);
}

void MenuSystem::drawSlider(sf::RenderWindow& window, sf::Font& font, const std::string& label, float value, float y, bool selected) {
    float winW = static_cast<float>(window.getSize().x);

    // Label
    sf::Text labelText;
    labelText.setFont(font);
    labelText.setString(label);
    labelText.setCharacterSize(20);
    labelText.setFillColor(selected ? Constants::UI_PRIMARY : Constants::UI_TEXT_DIM);
    labelText.setPosition(winW * 0.32f, y - 12.0f);
    window.draw(labelText);

    // Slider background
    float sliderWidth = 180.0f;
    float sliderHeight = 8.0f;
    float sliderX = winW * 0.55f;

    sf::RectangleShape sliderBg;
    sliderBg.setSize(sf::Vector2f(sliderWidth, sliderHeight));
    sliderBg.setFillColor(sf::Color(50, 55, 70));
    sliderBg.setPosition(sliderX, y);
    window.draw(sliderBg);

    // Slider fill
    sf::RectangleShape sliderFill;
    sliderFill.setSize(sf::Vector2f(sliderWidth * value, sliderHeight));
    sliderFill.setFillColor(selected ? Constants::UI_PRIMARY : Constants::UI_TEXT_DIM);
    sliderFill.setPosition(sliderX, y);
    window.draw(sliderFill);

    // Value percentage
    sf::Text valueText;
    valueText.setFont(font);
    valueText.setString(std::to_string(static_cast<int>(value * 100)) + "%");
    valueText.setCharacterSize(16);
    valueText.setFillColor(Constants::UI_TEXT_LIGHT);
    valueText.setPosition(sliderX + sliderWidth + 15.0f, y - 6.0f);
    window.draw(valueText);
}

void MenuSystem::drawCenteredText(sf::RenderWindow& window, sf::Font& font, const std::string& text, float y, int fontSize, sf::Color color) {
    float winW = static_cast<float>(window.getSize().x);

    sf::Text t;
    t.setFont(font);
    t.setString(text);
    t.setCharacterSize(fontSize);
    t.setFillColor(color);
    sf::FloatRect bounds = t.getLocalBounds();
    t.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
    t.setPosition(winW * 0.5f, y);
    window.draw(t);
}

void MenuSystem::drawAnimatedBackground(sf::RenderWindow& window) {
    // Scrolling pattern - can be expanded later
}
