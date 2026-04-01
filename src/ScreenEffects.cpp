#include "ScreenEffects.h"
#include <cmath>
#include <ctime>

ScreenEffects::ScreenEffects(sf::RenderWindow& window)
    : windowRef(window)
    , rng(static_cast<unsigned int>(std::time(nullptr)))
    , shakeIntensity(0.0f)
    , shakeOffset(0.0f, 0.0f)
    , flashColor(sf::Color::White)
    , flashTimer(0.0f)
    , flashDuration(0.0f)
    , transitioning(false)
    , fadingOut(true)
    , transitionProgress(0.0f)
    , transitionCallback(nullptr)
{
}

void ScreenEffects::update(float dt) {
    // Decay shake
    if (shakeIntensity > 0.1f) {
        shakeIntensity *= Constants::SCREEN_SHAKE_DECAY;

        std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
        shakeOffset.x = dist(rng) * shakeIntensity;
        shakeOffset.y = dist(rng) * shakeIntensity;
    } else {
        shakeIntensity = 0.0f;
        shakeOffset = sf::Vector2f(0.0f, 0.0f);
    }

    // Update flash
    if (flashTimer > 0.0f) {
        flashTimer -= dt;
    }

    // Update transition
    if (transitioning) {
        float speed = 1.0f / Constants::TRANSITION_DURATION;
        if (fadingOut) {
            transitionProgress += dt * speed;
            if (transitionProgress >= 1.0f) {
                transitionProgress = 1.0f;
                fadingOut = false;
                if (transitionCallback) {
                    transitionCallback();
                    transitionCallback = nullptr;
                }
            }
        } else {
            transitionProgress -= dt * speed;
            if (transitionProgress <= 0.0f) {
                transitionProgress = 0.0f;
                transitioning = false;
            }
        }
    }
}

void ScreenEffects::applyToView(sf::View& view) {
    if (shakeIntensity > 0.1f) {
        sf::Vector2f center = view.getCenter();
        view.setCenter(center + shakeOffset);
    }
}

void ScreenEffects::triggerShake(float intensity) {
    shakeIntensity = intensity;
}

void ScreenEffects::triggerFlash(sf::Color color, float duration) {
    flashColor = color;
    flashTimer = duration;
    flashDuration = duration;
}

void ScreenEffects::startTransition(bool fadeOut, std::function<void()> onComplete) {
    transitioning = true;
    fadingOut = fadeOut;
    transitionProgress = fadeOut ? 0.0f : 1.0f;
    transitionCallback = onComplete;
}

void ScreenEffects::renderOverlays(sf::RenderWindow& window) {
    float winW = static_cast<float>(window.getSize().x);
    float winH = static_cast<float>(window.getSize().y);

    // Flash overlay
    if (flashTimer > 0.0f) {
        float alpha = (flashTimer / flashDuration) * flashColor.a;
        sf::RectangleShape flash;
        flash.setSize(sf::Vector2f(winW, winH));
        flash.setFillColor(sf::Color(flashColor.r, flashColor.g, flashColor.b,
                                      static_cast<sf::Uint8>(alpha)));
        window.draw(flash);
    }

    // Transition overlay
    if (transitioning) {
        sf::Uint8 alpha = static_cast<sf::Uint8>(transitionProgress * 255);
        sf::RectangleShape transition;
        transition.setSize(sf::Vector2f(winW, winH));
        transition.setFillColor(sf::Color(0, 0, 0, alpha));
        window.draw(transition);
    }
}

void ScreenEffects::reset() {
    shakeIntensity = 0.0f;
    shakeOffset = sf::Vector2f(0.0f, 0.0f);
    flashTimer = 0.0f;
    transitioning = false;
    transitionProgress = 0.0f;
    transitionCallback = nullptr;
}
