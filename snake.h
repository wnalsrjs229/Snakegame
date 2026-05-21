#pragma once
#include "item.h"
#include "gate.h"

class Snake {
public:
    deque<Point> body;
    int dy = 0, dx = -1;
    int maxLength        = 3;
    int growthCount      = 0;
    int poisonCount      = 0;
    int gateCount        = 0;
    bool lastMoveUsedGate = false;

    void init(GameMap& gmap, int stage);
    void reset(GameMap& gmap, int stage);
    bool handleInput();
    bool move(GameMap& gmap, ItemManager& itemMgr, GateManager& gateMgr);
};
