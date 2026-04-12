#include "HUD.h"
#include "AssetManager.h"
#include "AssetKeys.h"
#include <cmath>
#include <algorithm>

HUD::HUD(sf::RenderWindow& window, AssetManager* assets)
    : window(window)
    , assets(assets)
    , displayScore(0.0f)
    , targetScore(0.0f)
    , bestScore(0.0f)
    , coins(0)
    , ammoCount(0)
    , maxAmmo(0)
    , unlimitedAmmo(false)
    , combo(0)
    , comboMultiplier(1.0f)
    , distance(0.0f)
    , speed(0.0f)
    , maxSpeed(Constants::MAX_OBSTACLE_SPEED)
    , comboFlashTimer(0.0f)
    , damageFlashTimer(0.0f)
    , pulseTimer(0.0f)
{
}

void HUD::update(float dt) {
    pulseTimer += dt;

    // Smooth score interpolation
    float scoreDiff = targetScore - displayScore;
    displayScore += scoreDiff * std::min(1.0f, dt * 10.0f);

    // Update flash timers
    if (comboFlashTimer > 0.0f) {
        comboFlashTimer -= dt;
    }
    if (damageFlashTimer > 0.0f) {
        damageFlashTimer -= dt;
    }

    // Update toasts
    for (auto it = toasts.begin(); it != toasts.end();) {
        it->lifetime -= dt;
        if (it->lifetime <= 0.0f) {
            it = toasts.erase(it);
        } else {
            // Slide-in animation
            if (it->slideOffset > 0.0f) {
                it->slideOffset = std::max(0.0f, it->slideOffset - Constants::TOAST_SLIDE_SPEED * dt);
            }
            ++it;
        }
    }

    // Update score popups
    for (auto it = scorePopups.begin(); it != scorePopups.end();) {
        it->lifetime -= dt;
        it->position.y -= 60.0f * dt;
        it->alpha = std::max(0.0f, it->alpha - 200.0f * dt);
        if (it->lifetime <= 0.0f) {
            it = scorePopups.erase(it);
        } else {
            ++it;
        }
    }

    // Update power-up timers
    for (auto it = activePowerUps.begin(); it != activePowerUps.end();) {
        if (it->remainingTime <= 0.0f) {
            it = activePowerUps.erase(it);
        } else {
            ++it;
        }
    }
}

void HUD::render(sf::RenderWindow& window, sf::Font& font) {
    renderDamageOverlay();
    renderScore();
    renderCoins();
    renderAmmo();
    renderCombo();
    renderSpeed();
    renderDistance();
    renderPowerUps();
    renderToasts();
    renderScorePopups();
}

void HUD::setScore(float score) {
    targetScore = score;
}

void HUD::setBestScore(float best) {
    bestScore = best;
}

void HUD::setCoins(int c) {
    coins = c;
}

void HUD::setAmmo(int count, int max, bool unlimited) {
    ammoCount = std::max(0, count);
    maxAmmo = std::max(0, max);
    unlimitedAmmo = unlimited;
}

void HUD::setCombo(int c, float mult) {
    if (c > combo) {
        triggerComboFlash();
    }
    combo = c;
    comboMultiplier = mult;
}

void HUD::triggerComboFlash() {
    comboFlashTimer = 0.5f;
}

void HUD::setDistance(float d) {
    distance = d;
}

void HUD::setMilestone(const std::string& m) {
    currentMilestone = m;
}

void HUD::addPowerUp(const std::string& name, sf::Color color, float duration) {
    // Check if already exists
    for (auto& pu : activePowerUps) {
        if (pu.name == name) {
            pu.remainingTime = duration;
            pu.maxTime = duration;
            return;
        }
    }
    activePowerUps.push_back({name, color, duration, duration});
}

void HUD::updatePowerUp(const std::string& name, float remainingTime) {
    for (auto& pu : activePowerUps) {
        if (pu.name == name) {
            pu.remainingTime = remainingTime;
            return;
        }
    }
}

void HUD::removePowerUp(const std::string& name) {
    activePowerUps.erase(
        std::remove_if(activePowerUps.begin(), activePowerUps.end(),
            [&name](const PowerUpDisplay& pu) { return pu.name == name; }),
        activePowerUps.end()
    );
}

void HUD::clearPowerUps() {
    activePowerUps.clear();
}

void HUD::setSpeed(float s, float max) {
    speed = s;
    maxSpeed = max;
}

void HUD::showToast(const std::string& message, sf::Color color) {
    Toast toast;
    toast.message = message;
    toast.color = color;
    toast.lifetime = Constants::TOAST_DISPLAY_TIME;
    toast.maxLifetime = Constants::TOAST_DISPLAY_TIME;
    toast.slideOffset = 200.0f;  // Slide in from right

    toasts.push_front(toast);
    while (toasts.size() > MAX_TOASTS) {
        toasts.pop_back();
    }
}

void HUD::showAchievementToast(const std::string& achievement) {
    showToast("Achievement: " + achievement, Constants::UI_SECONDARY);
}

void HUD::showMilestoneToast(const std::string& milestone) {
    showToast(milestone, Constants::UI_ACCENT);
}

void HUD::triggerDamageFlash() {
    damageFlashTimer = 0.3f;
}

void HUD::triggerScorePopup(const std::string& text, sf::Vector2f position) {
    ScorePopup popup;
    popup.text = text;
    popup.position = position;
    popup.lifetime = 1.0f;
    popup.alpha = 255.0f;
    scorePopups.push_back(popup);
}

void HUD::renderScore() {
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    float winW = static_cast<float>(window.getSize().x);

    // Score background panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(220.0f, 80.0f));
    panel.setFillColor(sf::Color(0, 0, 0, 120));
    panel.setPosition(Constants::HUD_MARGIN, Constants::HUD_MARGIN);
    window.draw(panel);

    // Score label
    sf::Text label;
    label.setFont(font);
    label.setString("SCORE");
    label.setCharacterSize(14);
    label.setFillColor(Constants::UI_TEXT_DIM);
    label.setPosition(Constants::HUD_MARGIN + 10.0f, Constants::HUD_MARGIN + 5.0f);
    window.draw(label);

    // Score value
    sf::Text scoreText;
    scoreText.setFont(font);
    scoreText.setString(std::to_string(static_cast<int>(displayScore)));
    scoreText.setCharacterSize(Constants::HUD_FONT_SIZE_LARGE);
    scoreText.setFillColor(Constants::UI_TEXT_LIGHT);
    scoreText.setStyle(sf::Text::Bold);
    scoreText.setPosition(Constants::HUD_MARGIN + 10.0f, Constants::HUD_MARGIN + 22.0f);
    window.draw(scoreText);

    // Best score
    sf::Text bestText;
    bestText.setFont(font);
    bestText.setString("Best: " + std::to_string(static_cast<int>(bestScore)));
    bestText.setCharacterSize(14);
    bestText.setFillColor(Constants::UI_TEXT_DIM);
    bestText.setPosition(Constants::HUD_MARGIN + 10.0f, Constants::HUD_MARGIN + 60.0f);
    window.draw(bestText);
}

void HUD::renderCoins() {
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    float winW = static_cast<float>(window.getSize().x);

    // Coin display in top-right
    sf::Text coinText;
    coinText.setFont(font);
    coinText.setString(std::to_string(coins));
    coinText.setCharacterSize(Constants::HUD_FONT_SIZE_MEDIUM);
    coinText.setFillColor(Constants::UI_SECONDARY);

    sf::FloatRect coinBounds = coinText.getLocalBounds();
    coinText.setPosition(winW - Constants::HUD_MARGIN - coinBounds.width - 30.0f, Constants::HUD_MARGIN + 5.0f);
    window.draw(coinText);

    // Coin icon (circle placeholder)
    sf::CircleShape coinIcon(10.0f);
    coinIcon.setFillColor(Constants::UI_SECONDARY);
    coinIcon.setPosition(winW - Constants::HUD_MARGIN - 25.0f, Constants::HUD_MARGIN + 10.0f);
    window.draw(coinIcon);
}

void HUD::renderAmmo() {
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);

    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(220.0f, 56.0f));
    panel.setFillColor(sf::Color(0, 0, 0, 120));
    panel.setPosition(Constants::HUD_MARGIN, Constants::HUD_MARGIN + 165.0f);
    window.draw(panel);

    sf::Text label;
    label.setFont(font);
    label.setString("AMMO");
    label.setCharacterSize(14);
    label.setFillColor(Constants::UI_TEXT_DIM);
    label.setPosition(Constants::HUD_MARGIN + 10.0f, Constants::HUD_MARGIN + 170.0f);
    window.draw(label);

    sf::Text value;
    value.setFont(font);
    value.setCharacterSize(Constants::HUD_FONT_SIZE_MEDIUM);
    value.setFillColor(unlimitedAmmo ? Constants::UI_SECONDARY : Constants::UI_TEXT_LIGHT);
    value.setStyle(sf::Text::Bold);
    if (unlimitedAmmo) {
        value.setString("INF");
    } else {
        value.setString(std::to_string(ammoCount) + "/" + std::to_string(maxAmmo));
    }
    value.setPosition(Constants::HUD_MARGIN + 10.0f, Constants::HUD_MARGIN + 186.0f);
    window.draw(value);

    sf::CircleShape bulletIcon(6.0f);
    bulletIcon.setFillColor(unlimitedAmmo ? Constants::UI_SECONDARY : sf::Color(245, 245, 245));
    bulletIcon.setOrigin(6.0f, 6.0f);
    bulletIcon.setPosition(Constants::HUD_MARGIN + 170.0f, Constants::HUD_MARGIN + 192.0f);
    window.draw(bulletIcon);
}

void HUD::renderCombo() {
    if (combo <= 1) return;

    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    float winW = static_cast<float>(window.getSize().x);

    // Combo panel
    sf::RectangleShape panel;
    panel.setSize(sf::Vector2f(150.0f, 60.0f));
    panel.setFillColor(sf::Color(0, 0, 0, 120));
    panel.setPosition(Constants::HUD_MARGIN, Constants::HUD_MARGIN + 95.0f);
    window.draw(panel);

    // Flash effect
    float flashAlpha = (comboFlashTimer > 0.0f) ? (comboFlashTimer * 400.0f) : 0.0f;
    if (flashAlpha > 0.0f) {
        sf::RectangleShape flash;
        flash.setSize(sf::Vector2f(150.0f, 60.0f));
        flash.setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(flashAlpha)));
        flash.setPosition(Constants::HUD_MARGIN, Constants::HUD_MARGIN + 95.0f);
        window.draw(flash);
    }

    // Combo count
    float scale = 1.0f + (comboFlashTimer > 0.0f ? comboFlashTimer * 0.5f : 0.0f);
    sf::Text comboText;
    comboText.setFont(font);
    comboText.setString(std::to_string(combo) + "x");
    comboText.setCharacterSize(static_cast<int>(Constants::HUD_FONT_SIZE_LARGE * scale));
    comboText.setFillColor(Constants::UI_PRIMARY);
    comboText.setStyle(sf::Text::Bold);
    comboText.setPosition(Constants::HUD_MARGIN + 15.0f, Constants::HUD_MARGIN + 98.0f);
    window.draw(comboText);

    // Multiplier
    sf::Text multText;
    multText.setFont(font);
    multText.setString("x" + std::to_string(static_cast<int>(comboMultiplier * 10) / 10.0f).substr(0, 3));
    multText.setCharacterSize(16);
    multText.setFillColor(Constants::UI_TEXT_DIM);
    multText.setPosition(Constants::HUD_MARGIN + 90.0f, Constants::HUD_MARGIN + 108.0f);
    window.draw(multText);
}

void HUD::renderSpeed() {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Speed bar at bottom
    float barWidth = 200.0f;
    float barHeight = 6.0f;
    float progress = std::min(1.0f, speed / maxSpeed);

    sf::RectangleShape barBg;
    barBg.setSize(sf::Vector2f(barWidth, barHeight));
    barBg.setFillColor(sf::Color(40, 45, 60, 180));
    barBg.setPosition(Constants::HUD_MARGIN, winH - Constants::HUD_MARGIN - barHeight - 30.0f);
    window.draw(barBg);

    // Color gradient from green to red based on speed
    sf::Uint8 red = static_cast<sf::Uint8>(progress * 255);
    sf::Uint8 green = static_cast<sf::Uint8>((1.0f - progress) * 255);
    sf::Color speedColor(red, green, 50);

    sf::RectangleShape barFill;
    barFill.setSize(sf::Vector2f(barWidth * progress, barHeight));
    barFill.setFillColor(speedColor);
    barFill.setPosition(Constants::HUD_MARGIN, winH - Constants::HUD_MARGIN - barHeight - 30.0f);
    window.draw(barFill);

    // Speed label
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    sf::Text speedLabel;
    speedLabel.setFont(font);
    speedLabel.setString("SPEED");
    speedLabel.setCharacterSize(12);
    speedLabel.setFillColor(Constants::UI_TEXT_DIM);
    speedLabel.setPosition(Constants::HUD_MARGIN, winH - Constants::HUD_MARGIN - barHeight - 48.0f);
    window.draw(speedLabel);
}

void HUD::renderDistance() {
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Distance display
    sf::Text distText;
    distText.setFont(font);
    distText.setString(std::to_string(static_cast<int>(distance)) + "m");
    distText.setCharacterSize(18);
    distText.setFillColor(Constants::UI_TEXT_DIM);
    sf::FloatRect distBounds = distText.getLocalBounds();
    distText.setPosition(winW * 0.5f - distBounds.width * 0.5f, Constants::HUD_MARGIN + 5.0f);
    window.draw(distText);

    // Milestone display
    if (!currentMilestone.empty()) {
        sf::Text milestoneText;
        milestoneText.setFont(font);
        milestoneText.setString(currentMilestone);
        milestoneText.setCharacterSize(14);
        milestoneText.setFillColor(Constants::UI_ACCENT);
        sf::FloatRect milestoneBounds = milestoneText.getLocalBounds();
        milestoneText.setPosition(winW * 0.5f - milestoneBounds.width * 0.5f, Constants::HUD_MARGIN + 28.0f);
        window.draw(milestoneText);
    }
}

void HUD::renderPowerUps() {
    if (activePowerUps.empty()) return;

    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    float winW = static_cast<float>(window.getSize().x);

    float startY = Constants::HUD_MARGIN + 50.0f;
    float iconSize = 40.0f;
    float spacing = 50.0f;

    for (size_t i = 0; i < activePowerUps.size(); ++i) {
        const auto& pu = activePowerUps[i];
        float x = winW - Constants::HUD_MARGIN - iconSize;
        float y = startY + i * spacing;

        // Background
        sf::RectangleShape bg;
        bg.setSize(sf::Vector2f(iconSize, iconSize));
        bg.setFillColor(sf::Color(0, 0, 0, 150));
        bg.setOutlineColor(pu.color);
        bg.setOutlineThickness(2.0f);
        bg.setPosition(x, y);
        window.draw(bg);

        // Progress overlay
        float progress = pu.remainingTime / pu.maxTime;
        sf::RectangleShape progressBar;
        progressBar.setSize(sf::Vector2f(iconSize, iconSize * (1.0f - progress)));
        progressBar.setFillColor(sf::Color(0, 0, 0, 180));
        progressBar.setPosition(x, y);
        window.draw(progressBar);

        // Icon letter
        sf::Text iconText;
        iconText.setFont(font);
        iconText.setString(pu.name.substr(0, 1));
        iconText.setCharacterSize(20);
        iconText.setFillColor(pu.color);
        iconText.setStyle(sf::Text::Bold);
        sf::FloatRect iconBounds = iconText.getLocalBounds();
        iconText.setOrigin(iconBounds.left + iconBounds.width * 0.5f, iconBounds.top + iconBounds.height * 0.5f);
        iconText.setPosition(x + iconSize * 0.5f, y + iconSize * 0.5f);
        window.draw(iconText);
    }
}

void HUD::renderToasts() {
    if (toasts.empty()) return;

    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    float startY = winH * 0.2f;
    float spacing = 50.0f;

    for (size_t i = 0; i < toasts.size(); ++i) {
        const auto& toast = toasts[i];
        float y = startY + i * spacing;
        float x = winW - 300.0f + toast.slideOffset;

        // Fade out near end of lifetime
        float alpha = std::min(1.0f, toast.lifetime / Constants::TOAST_FADE_TIME);

        // Background
        sf::RectangleShape bg;
        bg.setSize(sf::Vector2f(280.0f, 40.0f));
        bg.setFillColor(sf::Color(30, 35, 50, static_cast<sf::Uint8>(220 * alpha)));
        bg.setOutlineColor(sf::Color(toast.color.r, toast.color.g, toast.color.b, static_cast<sf::Uint8>(180 * alpha)));
        bg.setOutlineThickness(1.0f);
        bg.setPosition(x, y);
        window.draw(bg);

        // Text
        sf::Text text;
        text.setFont(font);
        text.setString(toast.message);
        text.setCharacterSize(16);
        text.setFillColor(sf::Color(toast.color.r, toast.color.g, toast.color.b, static_cast<sf::Uint8>(255 * alpha)));
        text.setPosition(x + 10.0f, y + 10.0f);
        window.draw(text);
    }
}

void HUD::renderDamageOverlay() {
    if (damageFlashTimer <= 0.0f) return;

    float alpha = damageFlashTimer * 400.0f;
    sf::RectangleShape overlay;
    overlay.setSize(sf::Vector2f(
        static_cast<float>(window.getSize().x),
        static_cast<float>(window.getSize().y)
    ));
    overlay.setFillColor(sf::Color(200, 0, 0, static_cast<sf::Uint8>(std::min(100.0f, alpha))));
    window.draw(overlay);
}

void HUD::renderScorePopups() {
    sf::Font& font = assets->get<sf::Font>(AssetKeys::MAIN_FONT);

    for (const auto& popup : scorePopups) {
        sf::Text text;
        text.setFont(font);
        text.setString(popup.text);
        text.setCharacterSize(20);
        text.setFillColor(sf::Color(Constants::UI_SECONDARY.r, Constants::UI_SECONDARY.g,
                                     Constants::UI_SECONDARY.b, static_cast<sf::Uint8>(popup.alpha)));
        text.setStyle(sf::Text::Bold);
        sf::FloatRect bounds = text.getLocalBounds();
        text.setOrigin(bounds.left + bounds.width * 0.5f, bounds.top + bounds.height * 0.5f);
        text.setPosition(popup.position);
        window.draw(text);
    }
}

sf::RectangleShape HUD::createProgressBar(float x, float y, float width, float height,
                                           float progress, sf::Color fillColor, sf::Color bgColor) {
    sf::RectangleShape bar;
    bar.setSize(sf::Vector2f(width * progress, height));
    bar.setFillColor(fillColor);
    bar.setPosition(x, y);
    return bar;
}
