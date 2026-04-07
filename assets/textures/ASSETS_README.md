# Game Assets - Deadlines Edge 2D

## Active Assets (Used in Game)

### Player & Characters
- `student_runner.png` - Player spritesheet (1024x512, animated frames)

### Obstacles (Intact)
- `obstacle_chair.png` - Wooden chair obstacle
- `obstacle_bench.png` - Wooden bench obstacle
- `obstacle_book.png` - Flying book obstacle

### Obstacles (Destroyed States)
- `destroyed_chair.png` - Broken chair debris sprite (64x64, transparent)
- `destroyed_bench.png` - Broken bench debris sprite (64x64, transparent)
- `destroyed_book.png` - Torn book pages sprite (64x64, transparent)

### Shooting Effects
- `bullet.png` - Bullet projectile sprite (32x32, transparent, yellow-orange glow)
- `muzzle_flash.png` - Gun muzzle flash effect (32x32, transparent)

### Particle Effects
- `explosion.png` - Explosion burst sprite (64x64, transparent, orange sparks)
- `hit_spark.png` - Impact spark effect (32x32, transparent, yellow-white)
- `smoke_puff.png` - Smoke cloud sprite (32x32, transparent, gray)

### Background Layers
- `bg_far.png` - Distant background layer (parallax)
- `bg_mid.png` - Middle background layer (parallax)
- `bg_near.png` - Near background layer (parallax)

## Legacy/Unused Assets (Can be removed)
- `player.png` - Old platformer player sprite (replaced by student_runner.png)
- `remnant.png` - Collectible from old platformer mode (not used in runner)

## Asset Loading
All assets are loaded in `AssetManager.cpp` via `loadAll()` method.
Asset keys are defined in `AssetKeys.h`.

## Texture Specifications
- **Transparent sprites**: All effect and destroyed object sprites use PNG alpha channel
- **Scaling**: Large sprites (1024x1024) are scaled down in code (typically 0.04-0.15x)
- **Format**: PNG format for all textures
- **Source**: Generated via Stitch MCP design tool

## Usage in Code

### Bullets (`bullet.png`, `muzzle_flash.png`)
- Loaded by: `ProjectileManager`
- Rendered when: Player clicks to shoot
- Location: `src/ProjectileManager.cpp`

### Explosions (`explosion.png`, `hit_spark.png`, `smoke_puff.png`)
- Loaded by: `ParticleSystem`
- Rendered when: Obstacles hit/destroyed
- Location: `src/ParticleSystem.cpp`

### Debris (`destroyed_*.png`)
- Loaded by: `Game` class
- Rendered when: Obstacle destroyed (HP reaches 0)
- Physics: Flies outward with gravity, rotates, fades out
- Location: `src/Game.cpp::spawnDebris()`
