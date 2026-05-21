#include "map.h"

vector<Point> GameMap::terrainWalls(int stage) const {
    vector<Point> walls;
    if (stage == 3) {
        int cy = MAP_SIZE / 2;
        int cx = MAP_SIZE / 2;
        for (int x = 1; x < MAP_SIZE - 1; x++)
            if (x != cx)
                walls.push_back({cy, x});
    } else if (stage == 4) {
        int cy = MAP_SIZE / 2, cx = MAP_SIZE / 2;
        int arm = MAP_SIZE / 8;
        for (int x = cx - arm; x <= cx + arm; x++)
            walls.push_back({cy, x});
        for (int y = cy - arm; y <= cy + arm; y++)
            if (y != cy)
                walls.push_back({y, cx});
    }
    return walls;
}

void GameMap::init(int stage) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            if ((y == 0 || y == MAP_SIZE-1) && (x == 0 || x == MAP_SIZE-1))
                cells[y][x] = 2;
            else if (y == 0 || y == MAP_SIZE-1 || x == 0 || x == MAP_SIZE-1)
                cells[y][x] = 1;
            else
                cells[y][x] = 0;
        }
    }
    for (const auto& p : terrainWalls(stage))
        cells[p.y][p.x] = 1;
}
