#include "ProjectileManager.h"
#include <cmath>

ProjectileManager::ProjectileManager()
    : fireCooldown(FIRE_COOLDOWN)
    , timeSinceLastFire(FIRE_COOLDOWN)  // Allow immediate first shot
    , bulletTexture(nullptr)
    , muzzleFlashTexture(nullptr)
    , muzzleFlashTimer(0.0f)
{
}

void ProjectileManager::setTextures(sf::Texture* bulletTex, sf::Texture* muzzleFlashTex) {
    bulletTexture = bulletTex;
    muzzleFlashTexture = muzzleFlashTex;
    
    if (bulletTexture) {
        bulletSprite.setTexture(*bulletTexture);
        // Center the origin
        sf::FloatRect bounds = bulletSprite.getLocalBounds();
        bulletSprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        // Scale down since generated images are large (1024x1024)
        bulletSprite.setScale(0.04f, 0.04f);
    }
}

void ProjectileManager::fire(const sf::Vector2f& origin, const sf::Vector2f& target, float speed) {
    if (timeSinceLastFire < fireCooldown) {
        return;  // Still on cooldown
    }

    // Calculate direction vector from origin to target
    sf::Vector2f direction = target - origin;
    float length = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    if (length < 1.0f) {
        return;  // Target too close, skip
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

    projectiles.push_back(bullet);
    timeSinceLastFire = 0.0f;
    
    // Trigger muzzle flash
    lastMuzzlePos = origin;
    muzzleFlashTimer = MUZZLE_FLASH_DURATION;
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

        // Deactivate if lifetime exceeded
        if (proj.lifetime >= proj.maxLifetime) {
            proj.active = false;
        }
    }

    // Remove inactive projectiles
    projectiles.remove_if([](const Projectile& p) { return !p.active; });
}

void ProjectileManager::render(sf::RenderWindow& window) {
    // Draw muzzle flash if active
    if (muzzleFlashTimer > 0.0f && muzzleFlashTexture) {
        sf::Sprite muzzleSprite(*muzzleFlashTexture);
        sf::FloatRect bounds = muzzleSprite.getLocalBounds();
        muzzleSprite.setOrigin(bounds.width / 2.0f, bounds.height / 2.0f);
        muzzleSprite.setScale(0.06f, 0.06f);
        muzzleSprite.setPosition(lastMuzzlePos);
        
        // Fade out
        float alpha = (muzzleFlashTimer / MUZZLE_FLASH_DURATION) * 255.0f;
        muzzleSprite.setColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
        window.draw(muzzleSprite);
    }

    for (const auto& proj : projectiles) {
        if (!proj.active) continue;

        if (bulletTexture) {
            // Use sprite texture
            bulletSprite.setPosition(proj.position);
            bulletSprite.setRotation(proj.rotation);
            window.draw(bulletSprite);
        } else {
            // Fallback: procedural rendering
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

        // Trail effect (always draw)
        sf::Vector2f trailPos = proj.position - proj.velocity * 0.015f;
        sf::CircleShape trail(BULLET_RADIUS * 0.5f);
        trail.setFillColor(sf::Color(255, 200, 80, 120));
        trail.setOrigin(BULLET_RADIUS * 0.5f, BULLET_RADIUS * 0.5f);
        trail.setPosition(trailPos);
        window.draw(trail);
    }
}

void ProjectileManager::reset() {
    projectiles.clear();
    timeSinceLastFire = FIRE_COOLDOWN;  // Allow immediate shot after reset
    muzzleFlashTimer = 0.0f;
}
