#pragma once

#include <cstdint>
#include <SFML/Graphics/Color.hpp>

// All tunable game constants live here.
// Grouped by system. Changing values here affects the entire game.

namespace Constants {

    // -- Physics (Box2D) --
    constexpr float PPM = 50.0f;                     // Pixels per meter (scale factor)
    constexpr float GRAVITY = 38.0f;                 // m/s^2 (snappy fall like Chrome Dino)
    constexpr float TIME_STEP = 1.0f / 60.0f;        // Fixed physics timestep (60 Hz)
    constexpr int   VELOCITY_ITERATIONS = 8;          // Box2D constraint solver iterations
    constexpr int   POSITION_ITERATIONS = 3;

    // -- Player (Ethan Calloway) --
    constexpr float PLAYER_WIDTH  = 2.5f;             // meters (125 pixels at PPM=50)
    constexpr float PLAYER_HEIGHT = 4.375f;           // meters (218.75 pixels at PPM=50) - 25% bigger
    constexpr float PLAYER_MOVE_SPEED = 6.0f;         // m/s max horizontal speed
    constexpr float PLAYER_JUMP_IMPULSE = 16.0f;      // m/s initial upward velocity - STRONGER
    constexpr float PLAYER_JUMP_HEIGHT_MULTIPLIER = 1.56f;  // Higher jump for runner mode (+30%)
    constexpr float PLAYER_JUMP_DISTANCE_MULTIPLIER = 1.625f; // Farther jump for runner mode (+30%)
    constexpr float PLAYER_GROUND_ACCEL = 0.15f;      // Velocity smoothing on ground (0-1, lower = heavier)
    constexpr float PLAYER_AIR_ACCEL = 0.08f;         // Velocity smoothing in air (less control)
    constexpr float PLAYER_DENSITY = 1.0f;            // kg/m^2
    constexpr float PLAYER_FRICTION = 0.3f;           // Surface friction coefficient
    constexpr float PLAYER_INVULNERABILITY_TIME = 1.5f; // Seconds of invulnerability after hit

    // -- Boundaries --
    constexpr float WALL_THICKNESS = 0.5f;            // meters (side walls and ceiling)
    constexpr float GROUND_THICKNESS = 1.0f;          // meters (thicker for visual weight)

    // -- Sprint --
    constexpr float PLAYER_SPRINT_MULTIPLIER = 1.6f;  // Speed multiplier when sprinting

    // -- Wall Jump --
    constexpr float WALL_JUMP_HORIZONTAL = 7.0f;      // Horizontal impulse away from wall
    constexpr float WALL_JUMP_VERTICAL = 8.0f;        // Vertical impulse during wall jump

    // -- Fixture user data identifiers --
    constexpr uintptr_t FIXTURE_BODY = 1;
    constexpr uintptr_t FIXTURE_FOOT_SENSOR = 2;
    constexpr uintptr_t FIXTURE_LEFT_WALL_SENSOR = 3;
    constexpr uintptr_t FIXTURE_RIGHT_WALL_SENSOR = 4;

    // -- Lane System (3 lanes) --
    constexpr int   LANE_COUNT = 3;                   // Left, Center, Right
    constexpr float LANE_WIDTH = 200.0f;              // Pixels between lane centers
    constexpr float LANE_SWITCH_SPEED = 15.0f;        // Speed of lane transitions

    // -- Combo System --
    constexpr float COMBO_TIMEOUT = 2.0f;             // Seconds before combo resets
    constexpr float NEAR_MISS_THRESHOLD = 30.0f;      // Pixels for near-miss detection
    constexpr int   NEAR_MISS_BONUS = 50;             // Points for near miss
    constexpr int   MAX_COMBO_MULTIPLIER = 10;        // Cap on multiplier

    // -- Power-Up System --
    constexpr float POWERUP_SPAWN_INTERVAL_MIN = 8.0f;
    constexpr float POWERUP_SPAWN_INTERVAL_MAX = 15.0f;
    constexpr float POWERUP_SHIELD_DURATION = 5.0f;
    constexpr float POWERUP_SLOWMO_DURATION = 4.0f;
    constexpr float POWERUP_SLOWMO_FACTOR = 0.5f;     // Time scale during slow-mo
    constexpr float POWERUP_DOUBLESCORE_DURATION = 8.0f;
    constexpr float POWERUP_MAGNET_DURATION = 6.0f;
    constexpr float POWERUP_UNLIMITED_BULLETS_DURATION = 6.0f;
    constexpr float POWERUP_MAGNET_RANGE = 300.0f;    // Pixel attraction radius

    // -- Projectile System --
    constexpr int   PROJECTILE_STARTING_AMMO = 18;
    constexpr int   PROJECTILE_MAX_AMMO = 30;
    constexpr int   PROJECTILE_AMMO_PICKUP = 5;              // Ammo granted per pickup
    constexpr float PROJECTILE_AMMO_DROP_DISTANCE_MIN = 100.0f; // meters
    constexpr float PROJECTILE_AMMO_DROP_DISTANCE_MAX = 200.0f; // meters

    // -- Coin System --
    constexpr float COIN_SPAWN_INTERVAL_MIN = 2.0f;
    constexpr float COIN_SPAWN_INTERVAL_MAX = 5.0f;
    constexpr int   COIN_VALUE = 1;
    constexpr float COIN_FLOAT_AMPLITUDE = 8.0f;      // Bobbing animation pixels
    constexpr float COIN_FLOAT_SPEED = 3.0f;          // Bobbing frequency

    // -- Difficulty Scaling --
    constexpr float DIFFICULTY_RAMP_RATE = 8.0f;      // Speed increase per second
    constexpr float MAX_OBSTACLE_SPEED = 830.0f;
    constexpr float MIN_SPAWN_DELAY = 0.5f;
    constexpr float DIFFICULTY_WAVE_DURATION = 30.0f; // Seconds between difficulty waves

    // -- Screen Effects --
    constexpr float SCREEN_SHAKE_DECAY = 0.9f;        // Per-frame decay
    constexpr float SCREEN_SHAKE_INTENSITY = 15.0f;   // Max shake pixels
    constexpr float TRANSITION_DURATION = 0.4f;       // Fade transition time

    // -- UI Colors (Campus Theme) --
    inline const sf::Color UI_PRIMARY{41, 98, 255};       // Bright blue
    inline const sf::Color UI_SECONDARY{255, 193, 7};     // Gold/amber
    inline const sf::Color UI_ACCENT{76, 175, 80};        // Success green
    inline const sf::Color UI_DANGER{244, 67, 54};        // Error red
    inline const sf::Color UI_TEXT_LIGHT{230, 235, 245};
    inline const sf::Color UI_TEXT_DIM{160, 172, 195};
    inline const sf::Color UI_BACKGROUND{18, 22, 32, 220};
    inline const sf::Color UI_OVERLAY{0, 0, 0, 180};

    // -- HUD Layout --
    constexpr float HUD_MARGIN = 20.0f;
    constexpr float HUD_PADDING = 10.0f;
    constexpr int   HUD_FONT_SIZE_LARGE = 36;
    constexpr int   HUD_FONT_SIZE_MEDIUM = 24;
    constexpr int   HUD_FONT_SIZE_SMALL = 16;

    // -- Toast Notifications --
    constexpr float TOAST_DISPLAY_TIME = 3.0f;
    constexpr float TOAST_FADE_TIME = 0.5f;
    constexpr float TOAST_SLIDE_SPEED = 200.0f;

    // -- Achievement Thresholds --
    constexpr int   ACHIEVEMENT_FIRST_RUN = 1;
    constexpr int   ACHIEVEMENT_SCORE_100 = 100;
    constexpr int   ACHIEVEMENT_SCORE_500 = 500;
    constexpr int   ACHIEVEMENT_SCORE_1000 = 1000;
    constexpr int   ACHIEVEMENT_COMBO_5 = 5;
    constexpr int   ACHIEVEMENT_COMBO_10 = 10;
    constexpr int   ACHIEVEMENT_COINS_50 = 50;
    constexpr int   ACHIEVEMENT_COINS_100 = 100;
}
