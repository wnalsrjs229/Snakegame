#include "gate.h"

void GateManager::clear() {
    gatesActive = false;
}

void GateManager::spawn(GameMap& gmap, int stage) {
    if (gatesActive) {
        gmap.set(gateList[0].pos.y, gateList[0].pos.x, 1);
        gmap.set(gateList[1].pos.y, gateList[1].pos.x, 1);
    }

    vector<Point> borderWalls;
    vector<Point> terrainW = gmap.terrainWalls(stage);

    for (int x = 1; x < MAP_SIZE-1; x++) {
        if (gmap.get(0, x) == 1)           borderWalls.push_back({0, x});
        if (gmap.get(MAP_SIZE-1, x) == 1)  borderWalls.push_back({MAP_SIZE-1, x});
    }
    for (int y = 1; y < MAP_SIZE-1; y++) {
        if (gmap.get(y, 0) == 1)           borderWalls.push_back({y, 0});
        if (gmap.get(y, MAP_SIZE-1) == 1)  borderWalls.push_back({y, MAP_SIZE-1});
    }

    vector<Point> allWalls = borderWalls;
    allWalls.insert(allWalls.end(), terrainW.begin(), terrainW.end());
    if (allWalls.size() < 2) return;

    int idx1, idx2;
    if (stage >= 3 && !terrainW.empty())
        idx1 = (int)borderWalls.size() + rand() % (int)terrainW.size();
    else
        idx1 = rand() % (int)allWalls.size();
    do { idx2 = rand() % (int)allWalls.size(); } while (idx2 == idx1);

    gateList[0] = {allWalls[idx1]};
    gateList[1] = {allWalls[idx2]};
    gmap.set(allWalls[idx1].y, allWalls[idx1].x, 7);
    gmap.set(allWalls[idx2].y, allWalls[idx2].x, 7);
    gatesActive = true;
}

pair<int,int> GateManager::exitDir(int edy, int edx, Point exitGate, const GameMap& gmap) const {
    int cands[4][2] = {
        { edy,  edx},
        { edx, -edy},
        {-edx,  edy},
        {-edy, -edx}
    };
    for (auto& c : cands) {
        int ny = exitGate.y + c[0];
        int nx = exitGate.x + c[1];
        if (ny >= 0 && ny < MAP_SIZE && nx >= 0 && nx < MAP_SIZE &&
            gmap.get(ny, nx) != 1 && gmap.get(ny, nx) != 2)
            return {c[0], c[1]};
    }
    return {edy, edx};
}
