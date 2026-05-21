#pragma once

#include <ncurses.h>
#include <deque>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <ctime>
#include <utility>
#include <algorithm>

using namespace std;
using namespace chrono;

#define MAP_SIZE         25
#define MAX_ITEMS         3
#define ITEM_INTERVAL_MS  3000
#define ITEM_LIFETIME_MS  10000
#define GATE_SPAWN_MS     10000

// 0=empty, 1=wall, 2=corner, 3=body, 4=head, 5=good, 6=bad, 7=gate, 8=trap

#define PAIR_WALL        1
#define PAIR_BODY        2
#define PAIR_HEAD        3
#define PAIR_GOOD        4
#define PAIR_BAD         5
#define PAIR_GATE        6
#define PAIR_TITLE       7
#define PAIR_DONE        8
#define PAIR_TEXT_GOOD   9
#define PAIR_TEXT_BAD   10
#define PAIR_TEXT_GATE  11
#define PAIR_TRAP       12
#define PAIR_BORDER     13
#define PAIR_TEXT_WHITE 14
#define PAIR_TEXT_TRAP  15

#define IPY   1
#define IPX   (MAP_SIZE * 2 + 2)
#define IPMID (IPX + 16)
#define IPH   12


struct Point { int y, x; };

struct Item {
    Point pos;
    int   type;
    steady_clock::time_point spawnTime;
    Point trapPos;
};

struct Gate {
    Point pos;
};
