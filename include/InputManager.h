#pragma once

#include <SFML/Window/Keyboard.hpp>
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

private:
    std::set<sf::Keyboard::Key> currentKeys;    // Lab: STL set — keys held this frame
    std::set<sf::Keyboard::Key> previousKeys;   // Lab: STL set — keys held last frame

    // Keys we actually care about tracking (avoids polling all 100+ keys)
    static const sf::Keyboard::Key trackedKeys[];
    static const int trackedKeyCount;
};
