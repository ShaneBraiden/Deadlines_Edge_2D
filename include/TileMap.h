#pragma once

#include <string>
#include <vector>
#include <SFML/Graphics.hpp>
#include <box2d/box2d.h>
#include "PhysicsWorld.h"
#include "ResourceManager.h"

// Forward declaration
struct TmxMapData;

// Tile types used in level files
// '.' = empty, '#' = solid, 'P' = player spawn, 'E' = exit, 'R' = remnant spawn
enum class TileType {
    Empty,
    Solid,
    PlayerSpawn,
    Exit,
    RemnantSpawn
};

// Tile-based level system using multi-level pointers (Tile**).
//
// WHY multi-level pointers:
//   The level grid is stored as a 2D array allocated with raw new:
//   TileType** grid  -- pointer to array of row pointers
//   grid[row]        -- pointer to array of TileType values
//   grid[row][col]   -- individual tile
//   This directly covers multi-level pointers and higher-dimension
//   pointer arithmetic lab requirements.
//
// Supports two loading modes:
//   1. Text format: loadFromFile() — legacy text-based level files
//   2. Tiled format: loadFromTmx() — .tmx XML maps with tileset textures
//
// Lab requirements covered:
//   - Multi-level pointers: TileType** grid, int** tileGids
//   - Higher dimension pointer arithmetic: *(*(grid + row) + col)
//   - Exception handling: file open/parse errors

class TileMap {
public:
    TileMap();
    ~TileMap();

    // Load a level from a text file (legacy format).
    // Throws std::runtime_error on failure.
    // Lab: exception handling
    void loadFromFile(const std::string& filepath);

    // Load a level from a Tiled .tmx file with tileset textures.
    // Throws std::runtime_error on failure.
    // Lab: exception handling
    void loadFromTmx(const std::string& filepath, ResourceManager<sf::Texture>& textures);

    // Create Box2D static bodies for all solid tiles.
    void createBodies(PhysicsWorld* physics);

    // Render all tiles to the window.
    void render(sf::RenderWindow& window, PhysicsWorld* physics);

    // Accessors
    int getRows() const { return rows; }
    int getCols() const { return cols; }
    float getTileSize() const { return tileSize; }

    // Get tile at position -- Lab: multi-level pointer access
    TileType getTile(int row, int col) const;

    // Set tile at position -- Lab: higher dimension pointer arithmetic
    void setTile(int row, int col, TileType type);

    // Find special tile positions (player spawn, exit, remnant spawns)
    sf::Vector2f getPlayerSpawn() const { return playerSpawn; }
    sf::Vector2f getExitPosition() const { return exitPosition; }
    const std::vector<sf::Vector2f>& getRemnantSpawns() const { return remnantSpawns; }

    // Free the grid memory
    void clear();

private:
    TileType** grid;        // Lab: multi-level pointer (pointer to array of row pointers)
    int** tileGids;         // Lab: multi-level pointer (tile GIDs for textured rendering)
    int rows;
    int cols;
    float tileSize;         // Size of each tile in meters

    // Tileset rendering data (populated by loadFromTmx)
    sf::Texture* tilesetTexture;    // Lab: single-level pointer (owned by ResourceManager)
    int tilesetColumns;             // Tiles per row in tileset image
    int tilesetTileWidth;           // Pixel width of one tileset tile
    int tilesetTileHeight;          // Pixel height of one tileset tile
    bool useTileset;                // True if loaded from .tmx with tileset

    sf::Vector2f playerSpawn;
    sf::Vector2f exitPosition;
    std::vector<sf::Vector2f> remnantSpawns;

    // Box2D bodies created for solid tiles (stored for cleanup)
    std::vector<b2Body*> tileBodies;

    // Allocate grid memory -- Lab: multi-level pointer allocation
    void allocateGrid(int numRows, int numCols);

    // Parse a character to a TileType
    static TileType charToTile(char c);
};
