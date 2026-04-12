#pragma once

// Compile-time string constants for all asset identifiers.
// Using these constants instead of raw strings prevents typos and enables
// IDE autocomplete for asset references throughout the codebase.
//
// Lab requirements covered:
//   - constexpr: compile-time constant expressions

namespace AssetKeys {

// === Active Assets (Runner Runtime) ===

// Player spritesheet (1024x512, 8 frames per row, 4 rows: idle, run, jump, fall)
constexpr const char* PLAYER_SHEET    = "student_runner";

// Obstacle textures
constexpr const char* OBSTACLE_CHAIR     = "obstacle_chair";
constexpr const char* OBSTACLE_BENCH     = "obstacle_bench";
constexpr const char* OBSTACLE_BOOK      = "obstacle_book";
constexpr const char* OBSTACLE_PROFESSOR = "obstacle_professor";  // 2 rows x 4 cols spritesheet
constexpr const char* ASSIGNMENT         = "assignment";          // Paper sheet thrown by professor

// Bullet texture (single sprite, muzzle flash and explosions are procedural)
constexpr const char* BULLET          = "bullet";

// Parallax background layers (repeated horizontally)
constexpr const char* BG_FAR          = "bg_far";
constexpr const char* BG_MID          = "bg_mid";
constexpr const char* BG_NEAR         = "bg_near";

// Fonts
constexpr const char* MAIN_FONT       = "main_font";

// === Dormant Assets (Registered but not wired into runner) ===

// Legacy player sprite (platformer mode)
constexpr const char* PLAYER_LEGACY   = "player_legacy";

// Remnant collectible sprite (platformer mode)
constexpr const char* REMNANT         = "remnant";

// Tileset for TMX maps (platformer mode)
constexpr const char* COLLEGE_TILESET = "college_tileset";

} // namespace AssetKeys
