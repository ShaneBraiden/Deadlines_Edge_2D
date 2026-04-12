#include "ProjectileManager.h"
#include <algorithm>
#include <cmath>

ProjectileManager::ProjectileManager()
    : fireCooldown(FIRE_COOLDOWN)
    , timeSinceLastFire(FIRE_COOLDOWN)  // Allow immediate first shot
    , bulletTexture(nullptr)
    , muzzleFlashTexture(nullptr)
    , muzzleFlashTimer(0.0f)
    , frameWidth(0)
    , frameHeight(0)
    , ammoCount(Constants::PROJECTILE_STARTING_AMMO)
    , maxAmmo(Constants::PROJECTILE_MAX_AMMO)
    , unlimitedAmmo(false)
{
}

void ProjectileManager::setTextures(sf::Texture* bulletTex, sf::Texture* muzzleFlashTex) {
    bulletTexture = bulletTex;
    muzzleFlashTexture = muzzleFlashTex;
    
    if (bulletTexture) {
        bulletSprite.setTexture(*bulletTexture);
        // Calculate frame dimensions from 4x2 spritesheet
        sf::Vector2u texSize = bulletTexture->getSize();
        frameWidth = texSize.x / FRAME_COLS;   // 4 columns
        frameHeight = texSize.y / FRAME_ROWS;  // 2 rows
        
        // Set initial frame (top-left)
        bulletSprite.setTextureRect(sf::IntRect(0, 0, frameWidth, frameHeight));
        bulletSprite.setOrigin(frameWidth / 2.0f, frameHeight / 2.0f);
        bulletSprite.setScale(0.375f, 0.375f);  // Smaller bullet sprite
    }
}

bool ProjectileManager::fire(const sf::Vector2f& origin, const sf::Vector2f& target, float speed) {
    if (timeSinceLastFire < fireCooldown) {
        return false;  // Still on cooldown
    }

    if (!unlimitedAmmo && ammoCount <= 0) {
        return false;
    }

    // Calculate direction vector from origin to target
    sf::Vector2f direction = target - origin;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length < 1.0f) {
        return false;  // Target too close, skip
    }

    // Normalize direction
    direction.x /= length;
    direction.y /= length;

    Projectile bullet;
    bullet.position = origin;
    bullet.velocity = sf::Vector2f(direction.x * speed, direction.y * speed);
    bullet.lifetime = 0.0f;
    bullet.maxLifetime = BULLET_LIFETIME;
    bullet.active = true;
    // Calculate rotation angle in degrees (sprite faces right by default)
    bullet.rotation = std::atan2(direction.y, direction.x) * 180.0f / 3.14159f;
    bullet.animTimer = 0.0f;
    bullet.currentFrame = 0;

    projectiles.push_back(bullet);
    timeSinceLastFire = 0.0f;
    if (!unlimitedAmmo) {
        ammoCount = std::max(0, ammoCount - 1);
    }
    
    // Trigger muzzle flash
    lastMuzzlePos = origin;
    muzzleFlashTimer = MUZZLE_FLASH_DURATION;
    return true;
}

void ProjectileManager::addAmmo(int amount) {
    if (amount <= 0) {
        return;
    }

    ammoCount = std::min(maxAmmo, ammoCount + amount);
}

void ProjectileManager::setUnlimitedAmmo(bool enabled) {
    unlimitedAmmo = enabled;
}

void ProjectileManager::update(float dt) {
    timeSinceLastFire += dt;
    
    // Update muzzle flash timer
    if (muzzleFlashTimer > 0.0f) {
        muzzleFlashTimer -= dt;
    }

    // Update all projectiles
    for (auto& proj : projectiles) {
        if (!proj.active) continue;

        proj.position += proj.velocity * dt;
        proj.lifetime += dt;

        // Update animation
        proj.animTimer += dt;
        if (proj.animTimer >= FRAME_TIME) {
            proj.animTimer -= FRAME_TIME;
            proj.currentFrame = (proj.currentFrame + 1) % FRAME_COUNT;
        }

        // Deactivate if lifetime exceeded
        if (proj.lifetime >= proj.maxLifetime) {
            proj.active = false;
        }
    }

    // Remove inactive projectiles
    projectiles.remove_if([](const Projectile& p) { return !p.active; });
}

void ProjectileManager::render(sf::RenderWindow& window) {
    // Draw procedural muzzle flash
    if (muzzleFlashTimer > 0.0f) {
        float alpha = (muzzleFlashTimer / MUZZLE_FLASH_DURATION);
        float flashSize = 20.0f * alpha;
        
        // Outer glow
        sf::CircleShape flash(flashSize);
        flash.setOrigin(flashSize, flashSize);
        flash.setPosition(lastMuzzlePos);
        flash.setFillColor(sf::Color(255, 200, 50, static_cast<sf::Uint8>(150 * alpha)));
        window.draw(flash);
        
        // Inner bright core
        sf::CircleShape core(flashSize * 0.5f);
        core.setOrigin(flashSize * 0.5f, flashSize * 0.5f);
        core.setPosition(lastMuzzlePos);
        core.setFillColor(sf::Color(255, 255, 200, static_cast<sf::Uint8>(255 * alpha)));
        window.draw(core);
    }

    for (const auto& proj : projectiles) {
        if (!proj.active) continue;

        if (bulletTexture && frameWidth > 0) {
            // Calculate frame position in 4x2 grid
            int col = proj.currentFrame % FRAME_COLS;  // 0-3
            int row = proj.currentFrame / FRAME_COLS;  // 0-1
            
            bulletSprite.setTextureRect(sf::IntRect(
                col * frameWidth,
                row * frameHeight,
                frameWidth,
                frameHeight
            ));
            bulletSprite.setPosition(proj.position);
            bulletSprite.setRotation(proj.rotation);
            window.draw(bulletSprite);
        } else {
            // Fallback: procedural bullet
            sf::CircleShape bulletShape(BULLET_RADIUS);
            bulletShape.setFillColor(sf::Color(255, 220, 100));
            bulletShape.setOrigin(BULLET_RADIUS, BULLET_RADIUS);
            bulletShape.setPosition(proj.position);
            window.draw(bulletShape);

            // Inner glow
            sf::CircleShape innerGlow(BULLET_RADIUS * 0.5f);
            innerGlow.setFillColor(sf::Color(255, 255, 200));
            innerGlow.setOrigin(BULLET_RADIUS * 0.5f, BULLET_RADIUS * 0.5f);
            innerGlow.setPosition(proj.position);
            window.draw(innerGlow);
        }

        // Trail effect
        for (int i = 1; i <= 3; ++i) {
            float trailAlpha = 100.0f - (i * 30.0f);
            float trailSize = BULLET_RADIUS * (1.0f - i * 0.2f);
            sf::Vector2f trailPos = proj.position - proj.velocity * (0.006f * i);
            
            sf::CircleShape trail(trailSize);
            trail.setFillColor(sf::Color(255, 180, 50, static_cast<sf::Uint8>(trailAlpha)));
            trail.setOrigin(trailSize, trailSize);
            trail.setPosition(trailPos);
            window.draw(trail);
        }
    }
}

void ProjectileManager::reset() {
    projectiles.clear();
    timeSinceLastFire = FIRE_COOLDOWN;  // Allow immediate shot after reset
    muzzleFlashTimer = 0.0f;
    ammoCount = Constants::PROJECTILE_STARTING_AMMO;
    unlimitedAmmo = false;
}
