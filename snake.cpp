#include "snake.h"

void Snake::init(GameMap& gmap, int stage) {
    int sy, sx;
    if (stage == 3)      { sy = 6;  sx = 12; }
    else if (stage == 4) { sy = 5;  sx = 5;  }
    else                 { sy = 12; sx = 12; }

    body.push_back({sy, sx});
    body.push_back({sy, sx-1});
    body.push_back({sy, sx-2});
    gmap.set(sy, sx,   4);
    gmap.set(sy, sx-1, 4);
    gmap.set(sy, sx-2, 3);
}

void Snake::reset(GameMap& gmap, int stage) {
    body.clear();
    dy = 0; dx = (stage == 4) ? 1 : -1;
    maxLength        = 3;
    growthCount      = 0;
    poisonCount      = 0;
    gateCount        = 0;
    lastMoveUsedGate = false;
    init(gmap, stage);
}

bool Snake::handleInput() {
    int key = getch();
    bool reverse = (key == KEY_UP    && dy ==  1) ||
                   (key == KEY_DOWN  && dy == -1) ||
                   (key == KEY_LEFT  && dx ==  1) ||
                   (key == KEY_RIGHT && dx == -1);
    if (reverse) return true;
    if (key == KEY_UP    && dy != 1  && dy != -1) { dy = -1; dx =  0; }
    if (key == KEY_DOWN  && dy != -1 && dy != 1)  { dy =  1; dx =  0; }
    if (key == KEY_LEFT  && dx != 1  && dx != -1) { dy =  0; dx = -1; }
    if (key == KEY_RIGHT && dx != -1 && dx != 1)  { dy =  0; dx =  1; }
    return false;
}

bool Snake::move(GameMap& gmap, ItemManager& itemMgr, GateManager& gateMgr) {
    lastMoveUsedGate = false;

    Point newHead = {body.front().y + dy, body.front().x + dx};
    int cell = gmap.get(newHead.y, newHead.x);

    if (gateMgr.gatesActive && cell == 7) {
        int enterIdx = (gateMgr.gateList[0].pos.y == newHead.y &&
                        gateMgr.gateList[0].pos.x == newHead.x) ? 0 : 1;
        int exitIdx  = 1 - enterIdx;

        pair<int,int> d = gateMgr.exitDir(dy, dx, gateMgr.gateList[exitIdx].pos, gmap);
        dy = d.first; dx = d.second;

        newHead = {gateMgr.gateList[exitIdx].pos.y + dy,
                   gateMgr.gateList[exitIdx].pos.x + dx};
        cell = gmap.get(newHead.y, newHead.x);
        gateCount++;
        lastMoveUsedGate = true;
    }

    if (cell == 1 || cell == 2 || cell == 3 || cell == 7 || cell == 8) return false;

    bool isGood = (cell == 5);
    bool isBad  = (cell == 6);

    if (isGood) growthCount++;
    if (isBad)  poisonCount++;

    if (isGood || isBad)
        itemMgr.removeAt(newHead.y, newHead.x, gmap);

    if (isBad) {
        Point extra = body.back();
        body.pop_back();
        gmap.set(extra.y, extra.x, 0);
    }

    if (!isGood) {
        Point tail = body.back();
        body.pop_back();
        gmap.set(tail.y, tail.x, 0);
    }

    gmap.set(body.front().y, body.front().x, 3);
    body.push_front(newHead);
    gmap.set(newHead.y, newHead.x, 4);

    if ((int)body.size() > maxLength) maxLength = (int)body.size();
    if ((int)body.size() < 3) return false;
    return true;
}
