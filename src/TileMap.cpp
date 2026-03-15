#include "TileMap.h"
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <iostream>

TileMap::TileMap()
    : grid(nullptr)
    , rows(0)
    , cols(0)
    , tileSize(1.0f)
{
}

TileMap::~TileMap() {
    clear();
}

void TileMap::allocateGrid(int numRows, int numCols) {
    // Lab: multi-level pointer allocation
    // Allocate array of row pointers
    grid = new TileType*[numRows];              // Lab: multi-level pointer (TileType**)

    // Allocate each row
    for (int r = 0; r < numRows; ++r) {
        // Lab: pointer arithmetic on grid (grid + r)
        *(grid + r) = new TileType[numCols];    // Lab: higher dimension pointer arithmetic

        // Initialize all tiles to empty
        for (int c = 0; c < numCols; ++c) {
            *(*(grid + r) + c) = TileType::Empty;  // Lab: higher dimension pointer arithmetic
        }
    }
}

void TileMap::clear() {
    if (grid) {
        for (int r = 0; r < rows; ++r) {
            delete[] *(grid + r);       // Lab: pointer arithmetic, delete row
        }
        delete[] grid;                  // Lab: delete row-pointer array
        grid = nullptr;
    }
    rows = 0;
    cols = 0;
    tileBodies.clear();
    remnantSpawns.clear();
}

TileType TileMap::charToTile(char c) {
    switch (c) {
        case '#': return TileType::Solid;
        case 'P': return TileType::PlayerSpawn;
        case 'E': return TileType::Exit;
        case 'R': return TileType::RemnantSpawn;
        case '.': default: return TileType::Empty;
    }
}

void TileMap::loadFromFile(const std::string& filepath) {
    // Clean up any previous level
    clear();

    std::ifstream file(filepath);
    if (!file.is_open()) {
        // Lab: exception handling
        throw std::runtime_error("TileMap: failed to open level file '" + filepath + "'");
    }

    // Read header: ROWS COLS TILE_SIZE
    if (!(file >> rows >> cols >> tileSize)) {
        throw std::runtime_error("TileMap: invalid header in '" + filepath + "'");
    }

    if (rows <= 0 || cols <= 0 || tileSize <= 0.0f) {
        throw std::runtime_error("TileMap: invalid dimensions in '" + filepath + "'");
    }

    // Allocate the grid -- Lab: multi-level pointer
    allocateGrid(rows, cols);

    // Read the tile grid
    // File rows are top-to-bottom, but Box2D Y is up, so we flip:
    // File row 0 = top of level = grid[rows-1]
    std::string line;
    std::getline(file, line); // consume rest of header line

    for (int r = 0; r < rows; ++r) {
        if (!std::getline(file, line)) {
            throw std::runtime_error("TileMap: not enough rows in '" + filepath + "'");
        }

        // File row r maps to grid[rows - 1 - r] (flip Y for Box2D)
        int gridRow = rows - 1 - r;

        for (int c = 0; c < cols && c < static_cast<int>(line.size()); ++c) {
            TileType type = charToTile(line[c]);

            // Lab: higher dimension pointer arithmetic
            *(*(grid + gridRow) + c) = type;

            // Record special positions (in world meters)
            float worldX = (c + 0.5f) * tileSize;
            float worldY = (gridRow + 0.5f) * tileSize;

            if (type == TileType::PlayerSpawn) {
                playerSpawn = sf::Vector2f(worldX, worldY);
            }
            else if (type == TileType::Exit) {
                exitPosition = sf::Vector2f(worldX, worldY);
            }
            else if (type == TileType::RemnantSpawn) {
                remnantSpawns.push_back(sf::Vector2f(worldX, worldY));
            }
        }
    }

    file.close();
}

TileType TileMap::getTile(int row, int col) const {
    if (!grid || row < 0 || row >= rows || col < 0 || col >= cols) {
        return TileType::Empty;
    }
    // Lab: multi-level pointer access
    return *(*(grid + row) + col);
}

void TileMap::setTile(int row, int col, TileType type) {
    if (!grid || row < 0 || row >= rows || col < 0 || col >= cols) {
        return;
    }
    // Lab: higher dimension pointer arithmetic
    *(*(grid + row) + col) = type;
}

void TileMap::createBodies(PhysicsWorld* physics) {
    if (!grid) return;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Lab: multi-level pointer access via pointer arithmetic
            TileType type = *(*(grid + r) + c);

            if (type == TileType::Solid) {
                float centerX = (c + 0.5f) * tileSize;
                float centerY = (r + 0.5f) * tileSize;
                float halfSize = tileSize / 2.0f;

                b2Body* body = physics->createStaticBody(
                    centerX, centerY, halfSize, halfSize
                );
                tileBodies.push_back(body);
            }
        }
    }
}

void TileMap::render(sf::RenderWindow& window, PhysicsWorld* physics) {
    if (!grid) return;

    sf::RectangleShape tileShape;
    float pixelSize = physics->toPixels(tileSize);
    tileShape.setSize(sf::Vector2f(pixelSize, pixelSize));
    tileShape.setOrigin(pixelSize / 2.0f, pixelSize / 2.0f);

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            // Lab: multi-level pointer traversal
            TileType type = *(*(grid + r) + c);

            if (type == TileType::Empty) continue;

            float worldX = (c + 0.5f) * tileSize;
            float worldY = (r + 0.5f) * tileSize;
            sf::Vector2f screenPos = physics->toScreen(b2Vec2(worldX, worldY));
            tileShape.setPosition(screenPos);

            switch (type) {
                case TileType::Solid:
                    tileShape.setFillColor(sf::Color(35, 35, 38));
                    tileShape.setOutlineColor(sf::Color(50, 50, 55));
                    tileShape.setOutlineThickness(1.0f);
                    break;
                case TileType::Exit:
                    tileShape.setFillColor(sf::Color(20, 50, 20));
                    tileShape.setOutlineColor(sf::Color(40, 80, 40));
                    tileShape.setOutlineThickness(1.0f);
                    break;
                case TileType::RemnantSpawn:
                case TileType::PlayerSpawn:
                    // Don't render spawn markers (invisible)
                    continue;
                default:
                    continue;
            }

            window.draw(tileShape);
        }
    }
}
