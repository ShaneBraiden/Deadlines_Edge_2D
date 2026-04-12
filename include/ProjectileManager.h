#pragma once

#include <SFML/Graphics.hpp>
#include <list>
#include <cmath>
#include "Constants.h"

// Manages projectiles (bullets) fired by the player.
// Bullets travel toward mouse cursor direction and can destroy obstacles.

struct Projectile {
    sf::Vector2f position;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
    bool active;
    float rotation;      // Angle in degrees for sprite rotation
    float animTimer;     // Animation timer
    int currentFrame;    // Current animation frame (0-7)
};

class ProjectileManager {
public:
    ProjectileManager();

    // Set the bullet texture (call after assets are loaded)
    void setTextures(sf::Texture* bulletTex, sf::Texture* muzzleFlashTex);

    // Fire a bullet from origin toward target direction
    bool fire(const sf::Vector2f& origin, const sf::Vector2f& target, float speed = 1200.0f);

    // Ammo management
    void addAmmo(int amount);
    void setUnlimitedAmmo(bool enabled);
    int getAmmo() const { return ammoCount; }
    int getMaxAmmo() const { return maxAmmo; }
    bool hasUnlimitedAmmo() const { return unlimitedAmmo; }

    // Update all projectiles
    void update(float dt);

    // Render all projectiles
    void render(sf::RenderWindow& window);

    // Get projectiles for collision checking (const access)
    const std::list<Projectile>& getProjectiles() const { return projectiles; }

    // Get projectiles for collision checking (mutable access for deactivating)
    std::list<Projectile>& getProjectilesMutable() { return projectiles; }

    // Clear all projectiles
    void reset();

private:
    std::list<Projectile> projectiles;
    float fireCooldown;
    float timeSinceLastFire;
    
    // Textures
    sf::Texture* bulletTexture;
    sf::Texture* muzzleFlashTexture;
    sf::Sprite bulletSprite;
    
    // Muzzle flash effect
    sf::Vector2f lastMuzzlePos;
    float muzzleFlashTimer;
    
    // Animation: 8 frames in 4x2 grid (4 columns, 2 rows)
    static constexpr int FRAME_COLS = 4;
    static constexpr int FRAME_ROWS = 2;
    static constexpr int FRAME_COUNT = 8;
    static constexpr float FRAME_TIME = 0.06f;  // Time per frame
    int frameWidth;
    int frameHeight;
    
    static constexpr float BULLET_RADIUS = 1.5f;
    static constexpr float BULLET_LIFETIME = 2.0f;
    static constexpr float FIRE_COOLDOWN = 0.15f;  // Time between shots
    static constexpr float MUZZLE_FLASH_DURATION = 0.05f;

    int ammoCount;
    int maxAmmo;
    bool unlimitedAmmo;
};
