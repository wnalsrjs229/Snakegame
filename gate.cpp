#include "gate.h"

/*
 * [clear] 게이트 상태를 초기화한다.
 * 스테이지가 새로 시작될 때 호출되며, gatesActive를 false로 설정해
 * 이전 게이트가 남아 있지 않도록 한다.
 */
void GateManager::clear() {
    gatesActive = false;
}

/*
 * [spawn] 새 게이트 쌍을 맵에 배치한다.
 *
 * 동작 흐름:
 *   1) 이미 게이트가 활성화되어 있으면, 기존 게이트 위치를 일반 벽(1)으로 되돌린다.
 *   2) 맵의 테두리 벽 좌표 목록(borderWalls)과 내부 지형 벽 목록(terrainW)을 수집한다.
 *   3) 스테이지 3 이상이고 내부 지형이 있을 경우, 게이트 1번은 반드시 내부 지형 위에
 *      배치되도록 idx1을 내부 지형 범위에서 선택한다.
 *      (스테이지 1·2는 테두리에서만 선택)
 *   4) 게이트 2번(idx2)은 전체 벽 목록에서 무작위로 선택하되, idx1과 겹치지 않아야 한다.
 *   5) 선택된 두 위치에 게이트(셀 값 7)를 표시하고 gatesActive를 true로 설정한다.
 */
void GateManager::spawn(GameMap& gmap, int stage) {
    // 기존 게이트가 있으면 맵에서 제거 (벽으로 복원)
    if (gatesActive) {
        gmap.set(gateList[0].pos.y, gateList[0].pos.x, 1);
        gmap.set(gateList[1].pos.y, gateList[1].pos.x, 1);
    }

    // 테두리 벽 좌표 수집 (상단·하단 가로줄, 좌측·우측 세로줄)
    vector<Point> borderWalls;
    const vector<Point> terrainW = gmap.terrainWalls(stage);  // 내부 지형 벽

    for (int x = 1; x < MAP_SIZE-1; x++) {
        if (gmap.get(0, x) == 1)           borderWalls.push_back({0, x});
        if (gmap.get(MAP_SIZE-1, x) == 1)  borderWalls.push_back({MAP_SIZE-1, x});
    }
    for (int y = 1; y < MAP_SIZE-1; y++) {
        if (gmap.get(y, 0) == 1)           borderWalls.push_back({y, 0});
        if (gmap.get(y, MAP_SIZE-1) == 1)  borderWalls.push_back({y, MAP_SIZE-1});
    }

    // 전체 후보 = 테두리 벽 + 내부 지형 벽
    vector<Point> allWalls = borderWalls;
    allWalls.insert(allWalls.end(), terrainW.begin(), terrainW.end());
    if (allWalls.size() < 2) return;  // 후보가 2개 미만이면 배치 불가

    int idx1, idx2;
    // 스테이지 3 이상이고 내부 지형이 있으면, 첫 번째 게이트를 내부 지형에서 선택
    // (allWalls에서 내부 지형은 borderWalls.size() 이후 인덱스에 위치)
    if (stage >= 3 && !terrainW.empty())
        idx1 = (int)borderWalls.size() + rand() % (int)terrainW.size();
    else
        idx1 = rand() % (int)allWalls.size();

    // 두 번째 게이트는 전체 벽에서 무작위로 선택, 첫 번째와 다른 위치여야 함
    do { idx2 = rand() % (int)allWalls.size(); } while (idx2 == idx1);

    gateList[0] = {allWalls[idx1]};
    gateList[1] = {allWalls[idx2]};
    gmap.set(allWalls[idx1].y, allWalls[idx1].x, 7);   // 게이트 표시
    gmap.set(allWalls[idx2].y, allWalls[idx2].x, 7);
    gatesActive = true;
}

/*
 * [exitDir] 뱀이 게이트를 통과할 때 출구 방향을 결정한다.
 *
 * 게이트가 벽 위에 있으므로, 출구 바로 옆 칸이 실제 이동 가능한 빈 칸이어야 한다.
 * 후보 방향을 우선순위 순서로 시도한다:
 *   1순위: 입구 진입 방향(edy, edx) 유지 (직진)
 *   2순위: 시계 방향 90도 회전
 *   3순위: 반시계 방향 90도 회전
 *   4순위: 정반대 방향 (U턴)
 *
 * 각 후보 방향으로 출구 게이트에서 한 칸 이동했을 때, 해당 칸이 맵 범위 안에 있고
 * 벽(1) 또는 코너 벽(2)이 아닌 경우 그 방향을 반환한다.
 *
 * 모든 후보가 막혀 있으면 진입 방향을 그대로 반환한다(fallback).
 *
 * 매개변수:
 *   edy, edx   - 뱀이 게이트로 진입할 때의 방향 벡터
 *   exitGate   - 출구 게이트의 맵 좌표
 *   gmap       - 현재 맵 상태 (벽 위치 확인용)
 */
pair<int,int> GateManager::exitDir(int edy, int edx, Point exitGate, const GameMap& gmap) const {
    // 후보 방향 4가지 : 직진, 우회전(CW 90°), 좌회전(CCW 90°), 유턴
    const int cands[4][2] = {
        { edy,  edx},   // 직진
        { edx, -edy},   // 시계방향 90도
        {-edx,  edy},   // 반시계방향 90도
        {-edy, -edx}    // 반대 방향
    };
    for (const auto& c : cands) {
        const int ny = exitGate.y + c[0];
        const int nx = exitGate.x + c[1];
        // 맵 범위 안에 있고 벽이 아닌 칸이면 해당 방향 선택
        if (ny >= 0 && ny < MAP_SIZE && nx >= 0 && nx < MAP_SIZE &&
            gmap.get(ny, nx) != 1 && gmap.get(ny, nx) != 2)
            return {c[0], c[1]};
    }
    // 모든 방향이 막혔을 경우 기존 방향 유지 (사실상 사망으로 이어짐)
    return {edy, edx};
}
