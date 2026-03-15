#include "InputManager.h"

// Keys the game actually uses — only these are polled each frame.
const sf::Keyboard::Key InputManager::trackedKeys[] = {
    sf::Keyboard::A,
    sf::Keyboard::D,
    sf::Keyboard::W,
    sf::Keyboard::S,
    sf::Keyboard::Left,
    sf::Keyboard::Right,
    sf::Keyboard::Up,
    sf::Keyboard::Down,
    sf::Keyboard::Space,
    sf::Keyboard::LShift,
    sf::Keyboard::Escape,
    sf::Keyboard::Enter,
    sf::Keyboard::Q,
    sf::Keyboard::P
};

const int InputManager::trackedKeyCount =
    sizeof(trackedKeys) / sizeof(trackedKeys[0]);

InputManager::InputManager() {}

void InputManager::update() {
    // Shift current frame to previous — Lab: STL set assignment
    previousKeys = currentKeys;
    currentKeys.clear();

    // Poll only the keys we care about — Lab: STL set insert
    for (int i = 0; i < trackedKeyCount; ++i) {
        if (sf::Keyboard::isKeyPressed(trackedKeys[i])) {
            currentKeys.insert(trackedKeys[i]);          // Lab: STL set insert
        }
    }
}

bool InputManager::isKeyHeld(sf::Keyboard::Key key) const {
    // Lab: STL set find + STL iterator comparison
    return currentKeys.find(key) != currentKeys.end();
}

bool InputManager::isKeyPressed(sf::Keyboard::Key key) const {
    // In current set but NOT in previous = just pressed this frame
    // Lab: STL set find + STL iterator comparison
    bool inCurrent  = currentKeys.find(key)  != currentKeys.end();
    bool inPrevious = previousKeys.find(key) != previousKeys.end();
    return inCurrent && !inPrevious;
}

bool InputManager::isKeyReleased(sf::Keyboard::Key key) const {
    // In previous set but NOT in current = just released this frame
    // Lab: STL set find + STL iterator comparison
    bool inCurrent  = currentKeys.find(key)  != currentKeys.end();
    bool inPrevious = previousKeys.find(key) != previousKeys.end();
    return !inCurrent && inPrevious;
}
