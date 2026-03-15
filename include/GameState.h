#pragma once

// Game states control which systems run during update/render.
//
// For Phase 2, the game starts directly in Play.
// Menu and Pause will be wired up in Phase 7.

enum class GameState {
    Menu,
    Play,
    Pause
};
