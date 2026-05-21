#pragma once
#include "common.h"

class GameMap {
public:
    int cells[MAP_SIZE][MAP_SIZE];

    vector<Point> terrainWalls(int stage) const;
    void init(int stage);
    int  get(int y, int x) const { return cells[y][x]; }
    void set(int y, int x, int v) { cells[y][x] = v; }
};
