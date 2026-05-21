#include "item.h"

// ── Food ──────────────────────────────────────────────────────────────────────

void Food::clear() { items.clear(); }
int  Food::count() const { return (int)items.size(); }

void Food::spawnSimple(GameMap& gmap) {
    auto empty = getEmptyCells(gmap);
    if (empty.empty()) return;
    Point p = empty[rand() % empty.size()];
    gmap.set(p.y, p.x, 5);
    items.push_back({p, 5, steady_clock::now(), {-1, -1}});
}

void Food::spawnWithTrap(GameMap& gmap) {
    auto empty = getEmptyCells(gmap);
    for (int i = (int)empty.size()-1; i > 0; i--)
        swap(empty[i], empty[rand() % (i+1)]);

    const int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    for (const auto& p : empty) {
        vector<Point> adj;
        for (const auto& d : dirs) {
            int ny = p.y+d[0], nx = p.x+d[1];
            if (ny >= 1 && ny < MAP_SIZE-1 && nx >= 1 && nx < MAP_SIZE-1 &&
                gmap.get(ny, nx) == 0)
                adj.push_back({ny, nx});
        }
        if (adj.empty()) continue;
        Point trap = adj[rand() % adj.size()];
        gmap.set(p.y, p.x, 5);
        gmap.set(trap.y, trap.x, 8);
        items.push_back({p, 5, steady_clock::now(), trap});
        return;
    }
}

void Food::expire(GameMap& gmap) {
    auto now = steady_clock::now();
    for (int i = (int)items.size()-1; i >= 0; i--) {
        if (duration_cast<milliseconds>(now - items[i].spawnTime).count() >= ITEM_LIFETIME_MS) {
            gmap.set(items[i].pos.y, items[i].pos.x, 0);
            if (items[i].trapPos.y >= 0)
                gmap.set(items[i].trapPos.y, items[i].trapPos.x, 0);
            items.erase(items.begin() + i);
        }
    }
}

int Food::findAt(int y, int x) const {
    for (int i = 0; i < (int)items.size(); i++)
        if (items[i].pos.y == y && items[i].pos.x == x)
            return i;
    return -1;
}

void Food::removeAt(int idx, GameMap& gmap) {
    if (items[idx].trapPos.y >= 0)
        gmap.set(items[idx].trapPos.y, items[idx].trapPos.x, 0);
    items.erase(items.begin() + idx);
}

// ── Poison ────────────────────────────────────────────────────────────────────

void Poison::clear() { items.clear(); }
int  Poison::count() const { return (int)items.size(); }

void Poison::spawn(GameMap& gmap) {
    auto empty = getEmptyCells(gmap);
    if (empty.empty()) return;
    Point p = empty[rand() % empty.size()];
    gmap.set(p.y, p.x, 6);
    items.push_back({p, 6, steady_clock::now(), {-1, -1}});
}

void Poison::expire(GameMap& gmap) {
    auto now = steady_clock::now();
    for (int i = (int)items.size()-1; i >= 0; i--) {
        if (duration_cast<milliseconds>(now - items[i].spawnTime).count() >= ITEM_LIFETIME_MS) {
            gmap.set(items[i].pos.y, items[i].pos.x, 0);
            items.erase(items.begin() + i);
        }
    }
}

int Poison::findAt(int y, int x) const {
    for (int i = 0; i < (int)items.size(); i++)
        if (items[i].pos.y == y && items[i].pos.x == x)
            return i;
    return -1;
}

void Poison::removeAt(int idx, GameMap& gmap) {
    items.erase(items.begin() + idx);
}

// ── ItemManager ───────────────────────────────────────────────────────────────

void ItemManager::clear() {
    food.clear();
    poison.clear();
}

void ItemManager::spawn(GameMap& gmap, int stage) {
    if (food.count() + poison.count() >= MAX_ITEMS) return;
    if (stage == 3)
        food.spawnWithTrap(gmap);
    else if (stage == 4)
        rand() % 2 == 0 ? food.spawnWithTrap(gmap) : poison.spawn(gmap);
    else
        rand() % 2 == 0 ? food.spawnSimple(gmap) : poison.spawn(gmap);
}

void ItemManager::expire(GameMap& gmap) {
    food.expire(gmap);
    poison.expire(gmap);
}

void ItemManager::removeAt(int y, int x, GameMap& gmap) {
    int fi = food.findAt(y, x);
    if (fi >= 0) { food.removeAt(fi, gmap); return; }
    int pi = poison.findAt(y, x);
    if (pi >= 0) { poison.removeAt(pi, gmap); }
}
