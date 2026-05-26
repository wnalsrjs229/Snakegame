#include "snake.h"

/*
 * [init] 스테이지에 따라 뱀의 초기 위치를 설정하고 맵에 표시한다.
 *
 * 스테이지별 시작 좌표 (머리 기준):
 *   스테이지 3 : (6, 12)  — 맵 위쪽 중앙 근처
 *   스테이지 4 : (5, 5)   — 맵 좌상단 근처
 *   그 외       : (12, 12) — 맵 정중앙
 *
 * 뱀은 머리(sy, sx), 몸1(sy, sx-1), 몸2(sy, sx-2) 총 3칸으로 시작한다.
 * 셀 값: 머리 = 4, 몸통 = 3 (헤더 common.h 참조)
 * body 덱(deque)에는 앞(front)이 머리, 뒤(back)가 꼬리다.
 */
void Snake::init(GameMap& gmap, int stage) {
    int sy, sx;
    if (stage == 3)      { sy = 6;  sx = 12; }
    else if (stage == 4) { sy = 5;  sx = 5;  }
    else                 { sy = 12; sx = 12; }

    // deque에 머리→몸1→몸2 순으로 추가
    body.push_back({sy, sx});      // 머리
    body.push_back({sy, sx-1});    // 몸통 1
    body.push_back({sy, sx-2});    // 몸통 2 (꼬리)

    // 맵에 표시 : 처음 두 칸은 모두 머리색(4)으로, 나머지는 몸통(3)으로
    gmap.set(sy, sx,   4);
    gmap.set(sy, sx-1, 4);
    gmap.set(sy, sx-2, 3);
}

/*
 * [reset] 스테이지 전환 시 뱀 상태를 완전히 초기화한다.
 *
 * - body를 비워 이전 뱀 잔상을 없앤다.
 * - 방향 초기화: 스테이지 4는 오른쪽(dx=1), 나머지는 왼쪽(dx=-1)
 * - 통계 카운터(최대 길이, 먹이·독·게이트 사용 횟수)를 0으로 리셋
 * - init()을 호출해 새 위치에 뱀을 그린다.
 */
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

/*
 * [handleInput] 키 입력을 읽어 뱀 방향을 바꾸거나 즉시 게임 오버를 유발한다.
 *
 * 반환값:
 *   true  = 반대 방향 키 입력 → 즉시 게임 오버
 *   false = 방향 변경 없음 또는 정상 방향 변경
 *
 * 규칙:
 *   - 반대 방향(180도 U턴)은 게임 규칙상 즉시 사망 처리.
 *     예) 현재 위로 이동 중(dy=-1)에 아래(KEY_DOWN, dy=+1) 입력 → game over
 *   - 같은 축(상하 or 좌우)을 이미 이동 중이면 방향 변경 무시.
 *     (대각선 이동 방지, deque 앞부분이 이미 겹침 방지)
 *   - 90도 방향 전환만 허용.
 */
bool Snake::handleInput() {
    int key = getch();

    // 현재 이동 방향의 정반대 키면 즉시 게임 오버
    bool reverse = (key == KEY_UP    && dy ==  1) ||
                   (key == KEY_DOWN  && dy == -1) ||
                   (key == KEY_LEFT  && dx ==  1) ||
                   (key == KEY_RIGHT && dx == -1);
    if (reverse) return true;

    // 90도 회전만 허용: 상하(dy축) 이동 중이면 좌우만, 좌우(dx축) 이동 중이면 상하만 변경 가능
    if (key == KEY_UP    && dy != 1  && dy != -1) { dy = -1; dx =  0; }
    if (key == KEY_DOWN  && dy != -1 && dy != 1)  { dy =  1; dx =  0; }
    if (key == KEY_LEFT  && dx != 1  && dx != -1) { dy =  0; dx = -1; }
    if (key == KEY_RIGHT && dx != -1 && dx != 1)  { dy =  0; dx =  1; }
    return false;
}

/*
 * [move] 현재 방향(dy, dx)으로 뱀을 한 칸 이동시킨다.
 *
 * 반환값:
 *   true  = 이동 성공 (생존)
 *   false = 충돌 또는 길이 부족 → 게임 오버
 *
 * ── 게이트 통과 처리 ──────────────────────────────────────────────────────────
 *   1) 다음 칸이 게이트(7)이고 게이트가 활성화된 경우:
 *      a) 진입한 게이트가 gateList[0]인지 [1]인지 판별한다.
 *      b) 반대쪽 게이트(출구)의 위치와 exitDir()로 계산한 출구 방향을 구한다.
 *      c) 실제 newHead = 출구 게이트에서 출구 방향으로 한 칸 이동한 위치
 *      d) gateCount 증가, lastMoveUsedGate = true
 *
 * ── 충돌 판정 ────────────────────────────────────────────────────────────────
 *   newHead의 셀 값이 다음 중 하나이면 충돌(게임 오버):
 *     1(벽), 2(코너), 3(몸통), 7(게이트 - 통과 실패 시), 8(트랩)
 *
 * ── 아이템 처리 ──────────────────────────────────────────────────────────────
 *   먹이(5) : growthCount++, 꼬리를 제거하지 않음 → 길이 1 증가
 *   독(6)   : poisonCount++, 꼬리를 2칸 제거 → 길이 1 감소 (일반 이동도 꼬리 1칸 제거)
 *   두 경우 모두 itemMgr.removeAt()으로 해당 아이템을 목록에서 삭제한다.
 *
 * ── 몸통 업데이트 순서 ────────────────────────────────────────────────────────
 *   1) 기존 머리 칸을 몸통(3)으로 바꾼다.
 *   2) newHead를 deque 앞에 push_front한다.
 *   3) newHead 칸을 머리(4)로 설정한다.
 *   4) 먹이가 아닌 경우에는 꼬리를 pop_back하고 해당 칸을 0으로 비운다.
 *      (독이면 추가로 한 칸 더 잘라냄 - 위 처리 이전에 별도로 실행됨)
 *
 * ── 길이 부족 처리 ────────────────────────────────────────────────────────────
 *   이동 후 body.size() < 3 이면 게임 오버(false 반환).
 */
bool Snake::move(GameMap& gmap, ItemManager& itemMgr, GateManager& gateMgr) {
    lastMoveUsedGate = false;

    // 다음에 이동할 머리 좌표 계산
    Point newHead = {body.front().y + dy, body.front().x + dx};
    int cell = gmap.get(newHead.y, newHead.x);

    // ── 게이트 통과 ──────────────────────────────────────────────────────────
    if (gateMgr.gatesActive && cell == 7) {
        // 진입 게이트 인덱스 판별 (0번 또는 1번)
        int enterIdx = (gateMgr.gateList[0].pos.y == newHead.y &&
                        gateMgr.gateList[0].pos.x == newHead.x) ? 0 : 1;
        int exitIdx  = 1 - enterIdx;  // 출구는 반대쪽

        // 출구 방향 계산 (직진 우선, 벽이면 회전)
        pair<int,int> d = gateMgr.exitDir(dy, dx, gateMgr.gateList[exitIdx].pos, gmap);
        dy = d.first; dx = d.second;

        // 출구 게이트에서 한 칸 더 나아간 위치가 실제 뱀 머리 위치
        newHead = {gateMgr.gateList[exitIdx].pos.y + dy,
                   gateMgr.gateList[exitIdx].pos.x + dx};
        cell = gmap.get(newHead.y, newHead.x);
        gateCount++;
        lastMoveUsedGate = true;
    }

    // ── 충돌 판정 ────────────────────────────────────────────────────────────
    // 벽(1), 코너(2), 내 몸통(3), 게이트(7, 통과 실패), 트랩(8) 충돌 → 사망
    if (cell == 1 || cell == 2 || cell == 3 || cell == 7 || cell == 8) return false;

    bool isGood = (cell == 5);   // 먹이
    bool isBad  = (cell == 6);   // 독

    if (isGood) growthCount++;
    if (isBad)  poisonCount++;

    // 아이템 칸이면 해당 아이템 데이터 삭제 (맵 셀은 뱀이 덮어씀)
    if (isGood || isBad)
        itemMgr.removeAt(newHead.y, newHead.x, gmap);

    // ── 독: 꼬리 1칸 추가 제거 ───────────────────────────────────────────────
    // 일반 이동 시 꼬리도 1칸 제거되므로 실질적으로 -1 효과가 생긴다
    if (isBad) {
        Point extra = body.back();
        body.pop_back();
        gmap.set(extra.y, extra.x, 0);
    }

    // ── 꼬리 이동 (먹이를 먹지 않은 경우만) ──────────────────────────────────
    // 먹이를 먹으면 꼬리를 유지해 길이 +1, 그 외에는 꼬리 제거로 길이 유지
    if (!isGood) {
        Point tail = body.back();
        body.pop_back();
        gmap.set(tail.y, tail.x, 0);  // 꼬리 칸을 빈 칸으로 비움
    }

    // ── 머리 전진 ────────────────────────────────────────────────────────────
    gmap.set(body.front().y, body.front().x, 3);  // 기존 머리 → 몸통
    body.push_front(newHead);                       // 새 머리 삽입
    gmap.set(newHead.y, newHead.x, 4);              // 새 머리 표시

    // 최대 길이 갱신
    if ((int)body.size() > maxLength) maxLength = (int)body.size();

    // 길이가 3 미만이 되면 게임 오버
    if ((int)body.size() < 3) return false;
    return true;
}
