#include "item.h"

// ══════════════════════════════════════════════════════════════════════════════
// Food (먹이 아이템, 셀 값 5 = **)
// 먹으면 뱀 길이가 1 증가하고 growthCount가 오른다.
// ══════════════════════════════════════════════════════════════════════════════

/* items 목록을 전부 비운다. 스테이지 초기화 시 호출. */
void Food::clear() { items.clear(); }

/* 현재 화면에 존재하는 먹이 개수를 반환한다. */
int  Food::count() const { return (int)items.size(); }

/*
 * [spawnSimple] 트랩 없이 먹이 하나를 임의의 빈 칸에 배치한다.
 * 스테이지 1·2에서 사용된다.
 *
 * 동작:
 *   1) 현재 맵의 빈 칸 목록을 가져온다.
 *   2) 그 중 하나를 무작위로 선택해 셀 값을 5(먹이)로 설정한다.
 *   3) items에 추가한다. trapPos는 (-1,-1)로 설정해 트랩 없음을 표시한다.
 */
void Food::spawnSimple(GameMap& gmap) {
    auto empty = getEmptyCells(gmap);
    if (empty.empty()) return;
    Point p = empty[rand() % empty.size()];
    gmap.set(p.y, p.x, 5);
    items.push_back({p, 5, steady_clock::now(), {-1, -1}});
}

/*
 * [spawnWithTrap] 먹이 옆에 트랩(셀 값 8 = !!)을 붙여서 배치한다.
 * 스테이지 3·4에서 사용된다. 트랩을 밟으면 즉시 게임 오버.
 *
 * 동작:
 *   1) 빈 칸 목록을 피셔-예이츠 셔플로 무작위 순서로 섞는다.
 *   2) 섞인 순서대로 후보 칸을 탐색하며, 해당 칸에 인접한 빈 칸(상하좌우)을 adj에 수집한다.
 *   3) adj가 비어 있지 않은 첫 번째 후보에 먹이와 트랩을 배치하고 즉시 반환한다.
 *      - 먹이(p)  : 셀 값 5
 *      - 트랩(trap): 인접 빈 칸 중 무작위 하나, 셀 값 8
 *   4) 트랩 위치는 trapPos에 저장해 두어, 먹이가 사라질 때 트랩도 함께 제거할 수 있게 한다.
 */
void Food::spawnWithTrap(GameMap& gmap) {
    auto empty = getEmptyCells(gmap);

    // 피셔-예이츠 셔플 : 뒤에서부터 무작위 인덱스와 교환
    for (int i = (int)empty.size()-1; i > 0; i--)
        swap(empty[i], empty[rand() % (i+1)]);

    const int dirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    for (const auto& p : empty) {
        // 후보 칸(p)의 상하좌우에서 내부 빈 칸만 수집
        vector<Point> adj;
        for (const auto& d : dirs) {
            int ny = p.y+d[0], nx = p.x+d[1];
            // 테두리(0, MAP_SIZE-1)는 제외하고 내부 빈 칸만 허용
            if (ny >= 1 && ny < MAP_SIZE-1 && nx >= 1 && nx < MAP_SIZE-1 &&
                gmap.get(ny, nx) == 0)
                adj.push_back({ny, nx});
        }
        if (adj.empty()) continue;   // 주변에 빈 칸이 없으면 다음 후보로

        // 인접 빈 칸 중 하나를 트랩 위치로 결정
        Point trap = adj[rand() % adj.size()];
        gmap.set(p.y, p.x, 5);        // 먹이
        gmap.set(trap.y, trap.x, 8);  // 트랩
        items.push_back({p, 5, steady_clock::now(), trap});
        return;   // 하나만 생성하고 종료
    }
}

/*
 * [expire] 수명(ITEM_LIFETIME_MS = 10초)이 지난 먹이를 맵과 목록에서 제거한다.
 * 매 프레임(게임 루프)마다 호출된다.
 *
 * 뒤에서 앞으로 순회하는 이유: erase()로 원소를 지울 때 앞 인덱스가 밀리면
 * 건너뛰는 원소가 생기므로, 역방향 순회로 이를 방지한다.
 * 트랩이 있는 먹이(trapPos.y >= 0)는 트랩 셀도 함께 0으로 초기화한다.
 */
void Food::expire(GameMap& gmap) {
    auto now = steady_clock::now();
    for (int i = (int)items.size()-1; i >= 0; i--) {
        if (duration_cast<milliseconds>(now - items[i].spawnTime).count() >= ITEM_LIFETIME_MS) {
            gmap.set(items[i].pos.y, items[i].pos.x, 0);  // 먹이 칸 초기화
            if (items[i].trapPos.y >= 0)
                gmap.set(items[i].trapPos.y, items[i].trapPos.x, 0);  // 트랩 칸 초기화
            items.erase(items.begin() + i);
        }
    }
}

/*
 * [findAt] (y, x) 위치에 있는 먹이의 인덱스를 반환한다.
 * 없으면 -1을 반환한다. 뱀이 해당 칸에 도달했을 때 아이템을 찾기 위해 사용.
 */
int Food::findAt(int y, int x) const {
    for (int i = 0; i < (int)items.size(); i++)
        if (items[i].pos.y == y && items[i].pos.x == x)
            return i;
    return -1;
}

/*
 * [removeAt] idx번 먹이를 목록에서 제거한다.
 * 트랩이 있는 먹이라면 트랩 셀도 함께 0으로 초기화한다.
 * (먹이 칸 자체는 Snake::move에서 뱀 머리로 덮어쓰므로 여기서 따로 초기화하지 않는다.)
 */
void Food::removeAt(int idx, GameMap& gmap) {
    if (items[idx].trapPos.y >= 0)
        gmap.set(items[idx].trapPos.y, items[idx].trapPos.x, 0);
    items.erase(items.begin() + idx);
}


// ══════════════════════════════════════════════════════════════════════════════
// Poison (독 아이템, 셀 값 6 = %%)
// 먹으면 꼬리 1칸이 추가로 잘려나가고 poisonCount가 오른다.
// 길이가 3 미만이 되면 게임 오버(snake.cpp에서 처리).
// ══════════════════════════════════════════════════════════════════════════════

/* items 목록을 전부 비운다. */
void Poison::clear() { items.clear(); }

/* 현재 화면에 존재하는 독 아이템 개수를 반환한다. */
int  Poison::count() const { return (int)items.size(); }

/*
 * [spawn] 임의의 빈 칸에 독 아이템(셀 값 6)을 하나 배치한다.
 * Food::spawnSimple과 동일한 방식이지만 타입이 6이고 trapPos가 없다.
 */
void Poison::spawn(GameMap& gmap) {
    auto empty = getEmptyCells(gmap);
    if (empty.empty()) return;
    Point p = empty[rand() % empty.size()];
    gmap.set(p.y, p.x, 6);
    items.push_back({p, 6, steady_clock::now(), {-1, -1}});
}

/*
 * [expire] 수명이 지난 독 아이템을 맵과 목록에서 제거한다.
 * Food::expire와 동일한 논리, 단 독에는 트랩이 없다.
 */
void Poison::expire(GameMap& gmap) {
    auto now = steady_clock::now();
    for (int i = (int)items.size()-1; i >= 0; i--) {
        if (duration_cast<milliseconds>(now - items[i].spawnTime).count() >= ITEM_LIFETIME_MS) {
            gmap.set(items[i].pos.y, items[i].pos.x, 0);
            items.erase(items.begin() + i);
        }
    }
}

/* (y, x)에 있는 독 아이템의 인덱스를 반환. 없으면 -1. */
int Poison::findAt(int y, int x) const {
    for (int i = 0; i < (int)items.size(); i++)
        if (items[i].pos.y == y && items[i].pos.x == x)
            return i;
    return -1;
}

/*
 * [removeAt] idx번 독 아이템을 목록에서 제거한다.
 * 맵 셀은 Snake::move에서 처리하므로 여기서는 목록만 정리한다.
 */
void Poison::removeAt(int idx, GameMap& gmap) {
    items.erase(items.begin() + idx);
}


// ══════════════════════════════════════════════════════════════════════════════
// ItemManager : Food와 Poison을 통합 관리하는 퍼사드(facade)
// ══════════════════════════════════════════════════════════════════════════════

/* 먹이와 독을 모두 초기화한다. 스테이지 시작 시 호출. */
void ItemManager::clear() {
    food.clear();
    poison.clear();
}

/*
 * [spawn] 아이템 하나를 생성한다.
 *
 * 아이템 총 개수(food + poison)가 MAX_ITEMS(3)를 넘으면 생성하지 않는다.
 *
 * 스테이지별 생성 규칙:
 *   스테이지 3 : 항상 트랩 동반 먹이(spawnWithTrap)만 생성
 *   스테이지 4 : 50% 확률로 트랩 동반 먹이, 50%로 독
 *   스테이지 1·2: 50% 확률로 일반 먹이, 50%로 독
 */
void ItemManager::spawn(GameMap& gmap, int stage) {
    if (food.count() + poison.count() >= MAX_ITEMS) return;

    if (stage == 3)
        food.spawnWithTrap(gmap);
    else if (stage == 4)
        rand() % 2 == 0 ? food.spawnWithTrap(gmap) : poison.spawn(gmap);
    else
        rand() % 2 == 0 ? food.spawnSimple(gmap) : poison.spawn(gmap);
}

/*
 * [expire] 수명이 지난 먹이와 독을 모두 제거한다.
 * 게임 루프마다 호출되어 맵 상태를 최신으로 유지한다.
 */
void ItemManager::expire(GameMap& gmap) {
    food.expire(gmap);
    poison.expire(gmap);
}

/*
 * [removeAt] (y, x) 위치의 아이템(먹이 또는 독)을 제거한다.
 * Snake::move에서 뱀 머리가 아이템 칸에 도달했을 때 호출된다.
 * 먹이를 먼저 확인하고, 없으면 독을 확인한다.
 */
void ItemManager::removeAt(int y, int x, GameMap& gmap) {
    int fi = food.findAt(y, x);
    if (fi >= 0) { food.removeAt(fi, gmap); return; }
    int pi = poison.findAt(y, x);
    if (pi >= 0) { poison.removeAt(pi, gmap); }
}
