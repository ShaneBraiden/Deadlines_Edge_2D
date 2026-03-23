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

const float InputManager::DOUBLE_CLICK_WINDOW = 0.3f;  // 300ms to double-click

InputManager::InputManager() 
    : spacePressCount(0)
    , timeSinceLastSpacePress(0.0f)
{
}

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

    // Update double-click timer (assuming ~60 FPS, so dt ≈ 0.0167f per frame)
    timeSinceLastSpacePress += 0.0167f;  // Approximate frame time
    
    // If we're outside the double-click window, reset the space press counter
    if (timeSinceLastSpacePress > DOUBLE_CLICK_WINDOW) {
        spacePressCount = 0;
    }

    // Detect space bar press and increment counter
    if (isKeyPressed(sf::Keyboard::Space)) {
        spacePressCount++;
        timeSinceLastSpacePress = 0.0f;  // Reset timer on each press
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

bool InputManager::isSpaceBarDoubleClicked() const {
    // Return true only when we detect exactly 2 presses within the double-click window
    // After the second press is detected, reset to 0 will happen on the next frame
    if (spacePressCount == 2 && isKeyPressed(sf::Keyboard::Space)) {
        return true;
    }
    return false;
}

