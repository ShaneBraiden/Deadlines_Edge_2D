#pragma once

#include <cstdint>

// All tunable game constants live here.
// Grouped by system. Changing values here affects the entire game.

namespace Constants {

    // -- Physics (Box2D) --
    constexpr float PPM = 50.0f;                     // Pixels per meter (scale factor)
    constexpr float GRAVITY = 25.0f;                 // m/s^2 (applied as -Y in Box2D Y-up coords)
    constexpr float TIME_STEP = 1.0f / 60.0f;        // Fixed physics timestep (60 Hz)
    constexpr int   VELOCITY_ITERATIONS = 8;          // Box2D constraint solver iterations
    constexpr int   POSITION_ITERATIONS = 3;

    // -- Player (Ethan Calloway) --
    constexpr float PLAYER_WIDTH  = 1.0f;             // meters (50 pixels at PPM=50)
    constexpr float PLAYER_HEIGHT = 1.8f;             // meters (90 pixels at PPM=50)
    constexpr float PLAYER_MOVE_SPEED = 6.0f;         // m/s max horizontal speed
    constexpr float PLAYER_JUMP_IMPULSE = 9.0f;       // m/s initial upward velocity
    constexpr float PLAYER_GROUND_ACCEL = 0.15f;      // Velocity smoothing on ground (0-1, lower = heavier)
    constexpr float PLAYER_AIR_ACCEL = 0.08f;         // Velocity smoothing in air (less control)
    constexpr float PLAYER_DENSITY = 1.0f;            // kg/m^2
    constexpr float PLAYER_FRICTION = 0.3f;           // Surface friction coefficient

    // -- Boundaries --
    constexpr float WALL_THICKNESS = 0.5f;            // meters (side walls and ceiling)
    constexpr float GROUND_THICKNESS = 1.0f;          // meters (thicker for visual weight)

    // -- Sprint --
    constexpr float PLAYER_SPRINT_MULTIPLIER = 1.6f;  // Speed multiplier when sprinting

    // -- Wall Jump --
    constexpr float WALL_JUMP_HORIZONTAL = 7.0f;      // Horizontal impulse away from wall
    constexpr float WALL_JUMP_VERTICAL = 8.0f;        // Vertical impulse during wall jump

    // -- Fixture user data identifiers --
    // Used by ContactListener to distinguish fixture types
    constexpr uintptr_t FIXTURE_BODY = 1;
    constexpr uintptr_t FIXTURE_FOOT_SENSOR = 2;
    constexpr uintptr_t FIXTURE_LEFT_WALL_SENSOR = 3;
    constexpr uintptr_t FIXTURE_RIGHT_WALL_SENSOR = 4;
}
