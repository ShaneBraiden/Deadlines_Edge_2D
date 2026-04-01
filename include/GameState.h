#pragma once

// Game states control which systems run during update/render.
// Expanded state machine to support full menu flow, shop, and achievements.

enum class GameState {
    Menu,           // Main menu with title, play, settings, quit
    Settings,       // Settings sub-menu (volume, controls)
    Play,           // Active gameplay
    Pause,          // Pause overlay with resume/settings/quit
    GameOver,       // Death screen with stats and retry
    Shop,           // Upgrade shop (spend coins)
    Achievements,   // Achievement gallery
    Transitioning   // Fade between states
};

// Sub-states for menu navigation
enum class MenuSelection {
    Play,
    Settings,
    Shop,
    Achievements,
    Quit,
    Resume,         // Pause menu
    QuitToMenu,     // Pause/GameOver menu
    Retry           // GameOver menu
};

// Settings sub-menu sections
enum class SettingsSection {
    Audio,
    Display,
    Controls,
    Back
};
