#include "map.h"

/*
 * [terrainWalls] 스테이지별 내부 지형 벽 좌표를 반환한다.
 *
 * 스테이지 3 : 맵 가로 중앙 줄(cy행)에 벽을 배치한다.
 *              단, 정중앙 열(cx)은 뚫려 있어 뱀이 통과할 수 있다.
 *              → 가운데 가로 벽 (수평 장벽, 중앙에 틈 1칸)
 *
 * 스테이지 4 : 맵 중심을 기준으로 ±arm 범위의 가로·세로 벽을 배치한다.
 *              가로 벽은 arm*2+1 칸, 세로 벽은 겹치는 중심(cy)을 제외하고 추가한다.
 *              → 십자(+) 모양 장벽
 *
 * 반환값 : 내부 지형 벽에 해당하는 Point(y, x) 리스트
 *          (스테이지 1·2는 내부 지형 없음 → 빈 벡터 반환)
 */
vector<Point> GameMap::terrainWalls(int stage) const {
    vector<Point> walls;
    if (stage == 3) {
        const int cy = MAP_SIZE / 2;   // 가로 벽이 놓일 행
        const int cx = MAP_SIZE / 2;   // 틈(갭)을 만들 열
        // x=1 ~ MAP_SIZE-2 범위를 순회하며, 중앙 열(cx)만 제외하고 벽 추가
        for (int x = 1; x < MAP_SIZE - 1; x++)
            if (x != cx)
                walls.push_back({cy, x});
    } else if (stage == 4) {
        const int cy = MAP_SIZE / 2, cx = MAP_SIZE / 2;
        const int arm = MAP_SIZE / 8;  // 십자 팔 길이 (중심에서 arm 칸씩 뻗어나감)
        // 수평 팔 : (cy, cx-arm) ~ (cy, cx+arm)
        for (int x = cx - arm; x <= cx + arm; x++)
            walls.push_back({cy, x});
        // 수직 팔 : (cy-arm, cx) ~ (cy+arm, cx), 단 cy(중심)는 수평에서 이미 추가
        for (int y = cy - arm; y <= cy + arm; y++)
            if (y != cy)
                walls.push_back({y, cx});
    }
    return walls;
}

/*
 * [init] 맵 전체를 초기화한다.
 *
 * 셀 값 규칙:
 *   0 = 빈 칸
 *   1 = 일반 벽 (테두리 변, 내부 지형)
 *   2 = 모서리 벽 (4개 꼭짓점)
 *
 * 처리 순서:
 *   1) 전체를 순회하며 테두리(y=0, y=MAX-1, x=0, x=MAX-1)를 벽(1)으로,
 *      4개 모서리를 코너 벽(2)으로, 나머지는 빈 칸(0)으로 설정한다.
 *   2) terrainWalls()가 반환하는 내부 지형 좌표를 추가로 벽(1)으로 설정한다.
 */
void GameMap::init(int stage) {
    for (int y = 0; y < MAP_SIZE; y++) {
        for (int x = 0; x < MAP_SIZE; x++) {
            // 4개 모서리는 값 2(코너 벽)
            if ((y == 0 || y == MAP_SIZE-1) && (x == 0 || x == MAP_SIZE-1))
                cells[y][x] = 2;
            // 나머지 테두리는 값 1(일반 벽)
            else if (y == 0 || y == MAP_SIZE-1 || x == 0 || x == MAP_SIZE-1)
                cells[y][x] = 1;
            // 내부는 빈 칸
            else
                cells[y][x] = 0;
        }
    }
    // 스테이지별 추가 내부 지형 적용
    for (const auto& p : terrainWalls(stage))
        cells[p.y][p.x] = 1;
}
