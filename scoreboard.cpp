#include "scoreboard.h"

// ══════════════════════════════════════════════════════════════════════════════
// Mission : 스테이지 클리어 조건 확인 (정적 메서드)
// 모든 조건이 동시에 충족되어야 true를 반환한다.
// 임계값은 scoreboard.h의 MISSION_* 매크로로 정의되어 있다.
// ══════════════════════════════════════════════════════════════════════════════

/*
 * 스테이지 1 클리어 조건:
 *   B(최대 길이) >= 10,  먹이 >= 5,  독 >= 2,  게이트 통과 >= 1
 */
bool Mission::checkStage1(int maxLen, int growth, int poison, int gate) {
    return maxLen  >= MISSION_S1_B    &&
           growth  >= MISSION_S1_G    &&
           poison  >= MISSION_S1_P    &&
           gate    >= MISSION_S1_GATE;
}

/*
 * 스테이지 2 클리어 조건:
 *   30초 생존 (elapsedMs >= 30000ms)
 */
bool Mission::checkStage2(long elapsedMs) {
    return elapsedMs >= 30000;
}

/*
 * 스테이지 3 클리어 조건:
 *   먹이 7개 이상 섭취 (growthCount >= MISSION_G3)
 */
bool Mission::checkStage3(int growth) {
    return growth >= MISSION_G3;
}

/*
 * 스테이지 4 클리어 조건 (종합):
 *   B(최대 길이) >= 10,  먹이 >= 7,  독 >= 3,  게이트 통과 >= 2,  30초 생존
 */
bool Mission::checkStage4(int maxLen, int growth, int poison, int gate, int elapsed) {
    return maxLen  >= MISSION_S4_B    &&
           growth  >= MISSION_S4_G    &&
           poison  >= MISSION_S4_P    &&
           gate    >= MISSION_S4_GATE &&
           elapsed >= MISSION_S4_TIME;
}


// ══════════════════════════════════════════════════════════════════════════════
// ScoreBoard : 맵 오른쪽에 현재 진행 상황과 미션 목표를 출력한다.
//
// 레이아웃 (common.h 매크로 기준):
//   IPX   = MAP_SIZE*2 + 2   → 맵 오른쪽 여백 이후 시작 열
//   IPMID = IPX + 16         → "현재 값" 열과 "목표 값" 열을 나누는 기준
//   IPY   = 1                → 시작 행
//   IPH   = 12               → 정보 패널 높이
//
// 왼쪽(IPX) : 현재 뱀 상태 (길이, 먹은 횟수 등)
// 오른쪽(IPMID) : 목표 수치 + 달성 여부 체크박스 [v] / [ ]
// ══════════════════════════════════════════════════════════════════════════════

/*
 * [drawStage1] 스테이지 1 정보 패널을 그린다.
 *
 * 표시 항목:
 *   B: 현재 길이 / 최대 길이   → 목표: B >= 10
 *   +: 먹이 섭취 횟수          → 목표: >= 5
 *   -: 독 섭취 횟수            → 목표: >= 2
 *   G: 게이트 통과 횟수        → 목표: >= 1
 */
void ScoreBoard::drawStage1(const Snake& snake) {
    pHeader(1);                 // "Stage 1" / "Mission" 헤더 출력
    const int row = IPY + 2;    // 헤더 아래 2줄부터 데이터 표시

    // 현재 상태 (왼쪽 열)
    mvprintw(row,   IPX, "B: %d / %d", (int)snake.body.size(), snake.maxLength);
    attron(COLOR_PAIR(PAIR_TEXT_GOOD));
    mvprintw(row+1, IPX, "+: %d", snake.growthCount);
    attroff(COLOR_PAIR(PAIR_TEXT_GOOD));
    attron(COLOR_PAIR(PAIR_TEXT_BAD));
    mvprintw(row+2, IPX, "-: %d", snake.poisonCount);
    attroff(COLOR_PAIR(PAIR_TEXT_BAD));
    attron(COLOR_PAIR(PAIR_TEXT_GATE));
    mvprintw(row+3, IPX, "G: %d", snake.gateCount);
    attroff(COLOR_PAIR(PAIR_TEXT_GATE));

    // 목표 수치 + 달성 체크박스 (오른쪽 열)
    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row,   IPMID, "B: %-5d  ", MISSION_S1_B);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.maxLength   >= MISSION_S1_B);

    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row+1, IPMID, "+: %-5d  ", MISSION_S1_G);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.growthCount >= MISSION_S1_G);

    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row+2, IPMID, "-: %-5d  ", MISSION_S1_P);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.poisonCount >= MISSION_S1_P);

    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row+3, IPMID, "G: %-5d  ", MISSION_S1_GATE);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.gateCount   >= MISSION_S1_GATE);
}

/*
 * [drawStage2] 스테이지 2 정보 패널을 그린다.
 *
 * 표시 항목:
 *   현재 뱀 길이, 남은 시간(초), 현재 속도 레벨
 *   목표: 30초 생존 (timeLeft == 0 이 되면 달성)
 *
 * timeLeft가 0이 되면 pCheck(true)로 체크 표시가 켜진다.
 */
void ScoreBoard::drawStage2(const Snake& snake, int timeLeft, int speedLv) {
    pHeader(2);
    const int row = IPY + 2;

    // 현재 상태
    mvprintw(row,   IPX, "Length:  %d",    (int)snake.body.size());
    mvprintw(row+1, IPX, "Time:    %ds",   timeLeft);
    mvprintw(row+2, IPX, "Speed:   Lv.%d", speedLv);

    // 목표
    mvprintw(row,   IPMID, "Survive 30s");
    mvprintw(row+1, IPMID, "Time  ");
    pCheck(timeLeft <= 0);   // 남은 시간 0이면 달성
}

/*
 * [drawStage3] 스테이지 3 정보 패널을 그린다.
 *
 * 표시 항목:
 *   +: 먹이 섭취 횟수  → 목표: >= 7
 */
void ScoreBoard::drawStage3(const Snake& snake) {
    pHeader(3);
    const int row = IPY + 2;

    // 현재 섭취 횟수 (초록색)
    attron(COLOR_PAIR(PAIR_TEXT_GOOD));
    mvprintw(row,   IPX, "+: %d", snake.growthCount);
    attroff(COLOR_PAIR(PAIR_TEXT_GOOD));

    // 목표 수치 + 달성 체크박스
    mvprintw(row,   IPMID, "Eat good items: %d", MISSION_G3);
    mvprintw(row+1, IPMID, "+  ");
    pCheck(snake.growthCount >= MISSION_G3);
}

/*
 * [drawStage4] 스테이지 4 정보 패널을 그린다. (스테이지 1+2 통합)
 *
 * 표시 항목:
 *   경과 시간(초), 속도 레벨
 *   B: 현재/최대 길이   → 목표: >= 10
 *   +: 먹이 횟수        → 목표: >= 7
 *   -: 독 횟수          → 목표: >= 3
 *   G: 게이트 통과 횟수 → 목표: >= 2
 *   30초 생존 여부      → 목표: elapsed >= 30
 */
void ScoreBoard::drawStage4(const Snake& snake, int elapsed, int speedLv) {
    pHeader(4);
    const int row = IPY + 2;

    // 현재 상태 (왼쪽 열)
    mvprintw(row,   IPX, "Time: %2ds  Lv.%d", elapsed, speedLv);
    mvprintw(row+1, IPX, "B: %d / %d", (int)snake.body.size(), snake.maxLength);
    attron(COLOR_PAIR(PAIR_TEXT_GOOD));
    mvprintw(row+2, IPX, "+: %d", snake.growthCount);
    attroff(COLOR_PAIR(PAIR_TEXT_GOOD));
    attron(COLOR_PAIR(PAIR_TEXT_BAD));
    mvprintw(row+3, IPX, "-: %d", snake.poisonCount);
    attroff(COLOR_PAIR(PAIR_TEXT_BAD));
    attron(COLOR_PAIR(PAIR_TEXT_GATE));
    mvprintw(row+4, IPX, "G: %d", snake.gateCount);
    attroff(COLOR_PAIR(PAIR_TEXT_GATE));

    // 목표 수치 + 달성 체크박스 (오른쪽 열)
    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row,   IPMID, "B: %-5d  ", MISSION_S4_B);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.maxLength   >= MISSION_S4_B);

    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row+1, IPMID, "+: %-5d  ", MISSION_S4_G);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.growthCount >= MISSION_S4_G);

    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row+2, IPMID, "-: %-5d  ", MISSION_S4_P);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.poisonCount >= MISSION_S4_P);

    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row+3, IPMID, "G: %-5d  ", MISSION_S4_GATE);
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(snake.gateCount   >= MISSION_S4_GATE);

    attron(COLOR_PAIR(PAIR_TEXT_WHITE));
    mvprintw(row+4, IPMID, "Survive 30s ");
    attroff(COLOR_PAIR(PAIR_TEXT_WHITE));
    pCheck(elapsed >= MISSION_S4_TIME);
}


// ══════════════════════════════════════════════════════════════════════════════
// 내부 헬퍼 메서드
// ══════════════════════════════════════════════════════════════════════════════

/*
 * [pCheck] 달성 여부를 체크박스 형태로 출력한다.
 *   done == true  → [v]  (초록 굵게)
 *   done == false → [ ]
 * printw를 사용해 현재 커서 위치 바로 뒤에 이어서 출력한다.
 */
void ScoreBoard::pCheck(bool done) {
    printw("[");
    if (done) {
        attron(COLOR_PAIR(PAIR_DONE) | A_BOLD);
        printw("v");
        attroff(COLOR_PAIR(PAIR_DONE) | A_BOLD);
    } else {
        printw(" ");
    }
    printw("]");
}

/*
 * [pHeader] 정보 패널 영역을 지우고 헤더 행을 출력한다.
 *
 * - IPY ~ IPY+IPH 범위를 공백으로 초기화해 이전 프레임 잔상을 없앤다.
 * - 첫 행에 "Stage N" (좌)와 "Mission" (우)를 노란색 굵게 표시한다.
 */
void ScoreBoard::pHeader(int stageNum) {
    // 정보 패널 영역 전체를 공백으로 지움
    for (int y = IPY; y < IPY + IPH; y++)
        mvprintw(y, IPX, "%-28s", "");   // 28칸을 공백으로 덮어씀

    // 헤더 출력
    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    mvprintw(IPY, IPX,   "Stage %d", stageNum);
    mvprintw(IPY, IPMID, "Mission");
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
}
