#pragma once
#include "map.h"

class GateManager {
public:
    Gate gateList[2];
    bool gatesActive = false;

    void clear();
    void spawn(GameMap& gmap, int stage);
    pair<int,int> exitDir(int edy, int edx, Point exitGate, const GameMap& gmap) const;
};
