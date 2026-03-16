#pragma once

#include <string>
#include <vector>

// Data parsed from a Tiled .tmx tileset reference.
struct TmxTileset {
    int firstGid;              // First global tile ID for this tileset
    int tileWidth;             // Pixel width of one tile
    int tileHeight;            // Pixel height of one tile
    int columns;               // Tiles per row in the tileset image
    int tileCount;             // Total tiles in tileset
    std::string imageSource;   // Relative path to tileset image
};

// A single tile layer parsed from .tmx.
struct TmxLayer {
    std::string name;
    int width;                 // Tiles wide
    int height;                // Tiles tall
    std::vector<int> data;     // Tile GIDs (0 = empty), row-major, top-to-bottom
};

// An object from a .tmx object layer (spawn points, triggers, etc).
struct TmxObject {
    std::string name;
    std::string type;          // "PlayerSpawn", "Exit", "RemnantSpawn"
    float x, y;                // Pixel position in Tiled (top-left origin, Y-down)
    float width, height;
};

// Complete parsed .tmx map data.
struct TmxMapData {
    int mapWidth;              // Tiles wide
    int mapHeight;             // Tiles tall
    int tileWidth;             // Pixels per tile
    int tileHeight;            // Pixels per tile
    std::vector<TmxTileset> tilesets;
    std::vector<TmxLayer> layers;
    std::vector<TmxObject> objects;
};

// Parses a Tiled .tmx file into TmxMapData.
// Throws std::runtime_error on parse failure.
// Lab: exception handling
class TmxLoader {
public:
    TmxMapData load(const std::string& filepath);

private:
    std::vector<int> parseCSV(const std::string& csv);
};
