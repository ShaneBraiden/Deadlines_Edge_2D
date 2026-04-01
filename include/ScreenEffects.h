#pragma once

#include <SFML/Graphics.hpp>
#include <random>
#include <functional>
#include "Constants.h"

// Manages screen shake, flash, transitions, and other visual effects.
class ScreenEffects {
public:
    ScreenEffects(sf::RenderWindow& window);

    // Update all effects
    void update(float dt);

    // Apply effects to a view (shake offset)
    void applyToView(sf::View& view);

    // Trigger effects
    void triggerShake(float intensity = Constants::SCREEN_SHAKE_INTENSITY);
    void triggerFlash(sf::Color color = sf::Color::White, float duration = 0.1f);
    void startTransition(bool fadeOut, std::function<void()> onComplete = nullptr);

    // Render overlay effects (flash, transitions)
    void renderOverlays(sf::RenderWindow& window);

    // Check if transitioning
    bool isTransitioning() const { return transitioning; }

    // Reset all effects
    void reset();

private:
    sf::RenderWindow& windowRef;
    std::mt19937 rng;

    // Shake state
    float shakeIntensity;
    sf::Vector2f shakeOffset;

    // Flash state
    sf::Color flashColor;
    float flashTimer;
    float flashDuration;

    // Transition state
    bool transitioning;
    bool fadingOut;
    float transitionProgress;
    std::function<void()> transitionCallback;
};
