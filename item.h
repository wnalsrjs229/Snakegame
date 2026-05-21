#pragma once
#include "map.h"

inline vector<Point> getEmptyCells(const GameMap& gmap) {
    vector<Point> empty;
    for (int y = 1; y < MAP_SIZE-1; y++)
        for (int x = 1; x < MAP_SIZE-1; x++)
            if (gmap.get(y, x) == 0)
                empty.push_back({y, x});
    return empty;
}

class Food {
public:
    vector<Item> items;

    void clear();
    int  count() const;
    void spawnSimple(GameMap& gmap);
    void spawnWithTrap(GameMap& gmap);
    void expire(GameMap& gmap);
    int  findAt(int y, int x) const;
    void removeAt(int idx, GameMap& gmap);
};

class Poison {
public:
    vector<Item> items;

    void clear();
    int  count() const;
    void spawn(GameMap& gmap);
    void expire(GameMap& gmap);
    int  findAt(int y, int x) const;
    void removeAt(int idx, GameMap& gmap);
};

class ItemManager {
public:
    Food   food;
    Poison poison;

    void clear();
    void spawn(GameMap& gmap, int stage);
    void expire(GameMap& gmap);
    void removeAt(int y, int x, GameMap& gmap);
};
