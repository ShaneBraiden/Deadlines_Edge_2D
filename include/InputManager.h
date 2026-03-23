#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/System/Clock.hpp>
#include <set>

// Tracks keyboard input state across frames using STL set.
//
// WHY a dedicated class:
//   Centralizes input logic. Supports "just pressed" and "just released"
//   queries that raw sf::Keyboard::isKeyPressed() can't provide alone.
//   This replaces the manual jumpPressed flag in Player.
//
// Lab requirements covered:
//   - STL set: std::set<sf::Keyboard::Key> for active key tracking
//   - STL iterators: explicit iteration over key sets during update

class InputManager {
public:
    InputManager();

    // Call once per frame before any gameplay logic.
    // Shifts current keys to previous, then polls the keyboard.
    void update();

    // Key is currently held down.
    bool isKeyHeld(sf::Keyboard::Key key) const;

    // Key was just pressed this frame (not held last frame).
    bool isKeyPressed(sf::Keyboard::Key key) const;

    // Key was just released this frame (held last frame, not this frame).
    bool isKeyReleased(sf::Keyboard::Key key) const;

    // Space bar double-click detection (for higher jumps).
    // Returns true on the frame where the second click is detected.
    bool isSpaceBarDoubleClicked() const;

private:
    std::set<sf::Keyboard::Key> currentKeys;    // Lab: STL set — keys held this frame
    std::set<sf::Keyboard::Key> previousKeys;   // Lab: STL set — keys held last frame

    // Double-click tracking for space bar
    int   spacePressCount;              // Number of presses in current window
    float timeSinceLastSpacePress;      // Time elapsed since last space key press (seconds)
    bool  spaceDoubleClickedThisFrame;  // Latched during update(), consumed as read-only query
    sf::Clock frameClock;               // Real elapsed time between update() calls
    static const float DOUBLE_CLICK_WINDOW;  // Time window for recognizing double-click (0.3 seconds)

    // Keys we actually care about tracking (avoids polling all 100+ keys)
    static const sf::Keyboard::Key trackedKeys[];
    static const int trackedKeyCount;
};
