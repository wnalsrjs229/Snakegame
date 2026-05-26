/*
 * Snake Game — 메인 진입점 및 게임 루프
 *
 * ┌─────────────────────────────────────────────────────────────────────────┐
 * │ 스테이지 구성                                                           │
 * │  Stage 1: 최대 길이 10, 먹이 5, 독 2, 게이트 1회 통과                  │
 * │  Stage 2: 30초 생존 — 5초마다 속도 증가                                │
 * │  Stage 3: 먹이 7개 섭취 — 먹이 옆에 트랩 동반                         │
 * │           맵: 가로 중앙 벽 (중심 1칸 틈)                               │
 * │  Stage 4: 종합 — 길이 10, 먹이 7, 독 3, 게이트 2, 30초 생존           │
 * │           맵: 중앙 십자(+) 장벽                                         │
 * │                                                                         │
 * │ 조작키                                                                  │
 * │   방향키         : 뱀 이동                                              │
 * │   현재 반대 방향 : 즉시 게임 오버                                       │
 * │                                                                         │
 * │ 컴파일                                                                  │
 * │   g++ -o snakegame snakegame.cpp map.cpp gate.cpp item.cpp              │
 * │          snake.cpp mission.cpp scoreboard.cpp -lncurses                 │
 * └─────────────────────────────────────────────────────────────────────────┘
 */

#include "scoreboard.h"

// ══════════════════════════════════════════════════════════════════════════════
// Renderer : ncurses를 이용한 화면 출력 담당
// 맵의 셀 값을 색상 문자로 변환하고, 게임 상태 메시지를 표시한다.
// ══════════════════════════════════════════════════════════════════════════════
class Renderer {
public:
    /*
     * [initColors] ncurses 색상 쌍(color pair)을 초기화한다.
     * 각 PAIR_* 상수는 common.h에 정의되어 있으며,
     * init_pair(번호, 전경색, 배경색) 형태로 설정한다.
     * 셀 렌더링(벽·몸통·아이템 등)과 텍스트 출력 색상 모두 여기서 정의한다.
     */
    void initColors() {
        start_color();
        init_pair(PAIR_WALL,       COLOR_WHITE,   COLOR_WHITE);    // 벽    : ## (흰색 블록)
        init_pair(PAIR_BODY,       COLOR_GREEN,   COLOR_GREEN);    // 몸통  : @@ (초록 블록)
        init_pair(PAIR_HEAD,       COLOR_GREEN,   COLOR_GREEN);    // 머리  : OO
        init_pair(PAIR_GOOD,       COLOR_CYAN,    COLOR_CYAN);     // 먹이  : ** (하늘색 블록)
        init_pair(PAIR_BAD,        COLOR_RED,     COLOR_RED);      // 독    : %% (빨간 블록)
        init_pair(PAIR_GATE,       COLOR_MAGENTA, COLOR_MAGENTA);  // 게이트: [] (자홍색 블록)
        init_pair(PAIR_TITLE,      COLOR_YELLOW,  COLOR_BLACK);    // 제목/헤더 텍스트 (노란색)
        init_pair(PAIR_DONE,       COLOR_GREEN,   COLOR_BLACK);    // 달성 체크 [v] (초록)
        init_pair(PAIR_TEXT_GOOD,  COLOR_CYAN,    COLOR_BLACK);    // 먹이 관련 텍스트
        init_pair(PAIR_TEXT_BAD,   COLOR_RED,     COLOR_BLACK);    // 독 관련 텍스트
        init_pair(PAIR_TEXT_GATE,  COLOR_MAGENTA, COLOR_BLACK);    // 게이트 관련 텍스트
        init_pair(PAIR_TRAP,       COLOR_YELLOW,  COLOR_YELLOW);   // 트랩  : !! (노란 블록)
        init_pair(PAIR_BORDER,     COLOR_CYAN,    COLOR_BLACK);    // (예비)
        init_pair(PAIR_TEXT_WHITE, COLOR_WHITE,   COLOR_BLACK);    // 일반 흰색 텍스트
        init_pair(PAIR_TEXT_TRAP,  COLOR_YELLOW,  COLOR_BLACK);    // 트랩 관련 텍스트
    }

    /*
     * [drawMap] 맵 전체(MAP_SIZE × MAP_SIZE)를 ncurses로 출력한다.
     *
     * 셀 값 → 표시 문자 대응:
     *   1, 2 → ## (벽/코너, 흰색)
     *   3    → @@ (뱀 몸통, 초록)
     *   4    → OO (뱀 머리, 초록)
     *   5    → ** (먹이, 하늘색)
     *   6    → %% (독, 빨간색)
     *   7    → [] (게이트, 자홍색)
     *   8    → !! (트랩, 노란색)
     *   0    →    (빈 칸, 공백 2자)
     *
     * 가로 좌표를 x*2로 출력하는 이유: 각 셀이 2문자 너비여서
     * 화면에서 정사각형처럼 보이게 한다.
     */
    void drawMap(const GameMap& gmap) {
        for (int y = 0; y < MAP_SIZE; y++) {
            for (int x = 0; x < MAP_SIZE; x++) {
                switch (gmap.get(y, x)) {
                case 1: case 2:
                    attron(COLOR_PAIR(PAIR_WALL) | A_BOLD);
                    mvprintw(y, x*2, "##");
                    attroff(COLOR_PAIR(PAIR_WALL) | A_BOLD);
                    break;
                case 3:
                    attron(COLOR_PAIR(PAIR_BODY) | A_BOLD);
                    mvprintw(y, x*2, "@@");
                    attroff(COLOR_PAIR(PAIR_BODY) | A_BOLD);
                    break;
                case 4:
                    attron(COLOR_PAIR(PAIR_HEAD));
                    mvprintw(y, x*2, "OO");
                    attroff(COLOR_PAIR(PAIR_HEAD));
                    break;
                case 5:
                    attron(COLOR_PAIR(PAIR_GOOD) | A_BOLD);
                    mvprintw(y, x*2, "**");
                    attroff(COLOR_PAIR(PAIR_GOOD) | A_BOLD);
                    break;
                case 6:
                    attron(COLOR_PAIR(PAIR_BAD) | A_BOLD);
                    mvprintw(y, x*2, "%%");
                    attroff(COLOR_PAIR(PAIR_BAD) | A_BOLD);
                    break;
                case 7:
                    attron(COLOR_PAIR(PAIR_GATE) | A_BOLD);
                    mvprintw(y, x*2, "[]");
                    attroff(COLOR_PAIR(PAIR_GATE) | A_BOLD);
                    break;
                case 8:
                    attron(COLOR_PAIR(PAIR_TRAP) | A_BOLD);
                    mvprintw(y, x*2, "!!");
                    attroff(COLOR_PAIR(PAIR_TRAP) | A_BOLD);
                    break;
                default:
                    mvprintw(y, x*2, "  ");   // 빈 칸
                    break;
                }
            }
        }
    }

    /*
     * [showGameOver] "Game Over" 메시지를 출력하고 키 입력을 기다린다.
     * timeout(-1)로 블로킹 입력 대기 후 게임을 종료한다.
     */
    void showGameOver() {
        attron(COLOR_PAIR(PAIR_TEXT_BAD) | A_BOLD);
        mvprintw(MAP_SIZE + 1, 0, "Game Over! Press any key...");
        attroff(COLOR_PAIR(PAIR_TEXT_BAD) | A_BOLD);
        refresh();
        timeout(-1);   // 블로킹 모드로 전환 (키 입력 대기)
        getch();
    }

    /*
     * [showStageClear] 스테이지 클리어 메시지를 출력하고 키 입력을 기다린다.
     * 마지막 스테이지(4)이면 "Game Clear!", 아니면 다음 스테이지 안내를 표시한다.
     * 키 입력 후 다시 timeout(0) 논블로킹 모드로 복귀한다.
     */
    void showStageClear(int stage) {
        attron(COLOR_PAIR(PAIR_DONE) | A_BOLD);
        if (stage == 4)
            mvprintw(MAP_SIZE + 1, 0, "Game Clear! Press any key...");
        else
            mvprintw(MAP_SIZE + 1, 0, "Stage %d Clear! Press any key for Stage %d...", stage, stage + 1);
        attroff(COLOR_PAIR(PAIR_DONE) | A_BOLD);
        refresh();
        timeout(-1); getch(); timeout(0);   // 키 입력 대기 → 논블로킹 복귀
    }
};


// ══════════════════════════════════════════════════════════════════════════════
// Game : 전체 게임 흐름을 관장하는 최상위 클래스
//
// 멤버:
//   gmap       - 맵 상태 (셀 배열)
//   snake      - 뱀 상태 (몸통 위치, 방향, 통계)
//   itemMgr    - 먹이·독 아이템 생성/소멸 관리
//   gateMgr    - 게이트 배치 및 통과 방향 계산
//   renderer   - 화면 출력
//   scoreboard - 우측 정보 패널 출력 + 미션 달성 판정
// ══════════════════════════════════════════════════════════════════════════════
class Game {
public:
    GameMap     gmap;
    Snake       snake;
    ItemManager itemMgr;
    GateManager gateMgr;
    Renderer    renderer;
    ScoreBoard  scoreboard;

    /*
     * [run] 게임 전체를 실행한다.
     *
     * 1) selectStage()로 시작 스테이지를 선택받는다.
     * 2) 선택한 스테이지부터 4까지 순서대로 실행한다.
     * 3) 각 스테이지 클리어 시 showStageClear() 후 다음 스테이지로 진행한다.
     * 4) 게임 오버 시 showGameOver() 후 즉시 종료한다.
     */
    void run() {
        const int startStage = selectStage();
        timeout(0);   // 논블로킹 입력 모드 (getch()가 즉시 반환)

        for (int stage = startStage; stage <= 4; stage++) {
            bool cleared = false;
            switch (stage) {
            case 1: cleared = runStage1(); break;
            case 2: cleared = runStage2(); break;
            case 3: cleared = runStage3(); break;
            case 4: cleared = runStage4(); break;
            }
            if (!cleared) { renderer.showGameOver(); return; }
            renderer.showStageClear(stage);
        }
    }

private:
    /*
     * [reset] 스테이지 시작 전 아이템·게이트·맵·뱀을 모두 초기 상태로 되돌린다.
     * 각 모듈의 clear()/init()/reset()을 순서대로 호출한다.
     */
    void reset(int stage) {
        itemMgr.clear();
        gateMgr.clear();
        gmap.init(stage);
        snake.reset(gmap, stage);
    }

    /*
     * [selectStage] 시작 스테이지 선택 화면을 표시하고 '1'~'4' 입력을 기다린다.
     * 유효한 키가 입력될 때까지 루프를 돌며, 해당 숫자를 반환한다.
     */
    int selectStage() {
        clear();
        attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
        mvprintw(2, 2, "Snake Game");
        attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
        mvprintw(4, 2, "Select stage:");
        mvprintw(5, 2, "  1 - Stage 1");
        mvprintw(6, 2, "  2 - Stage 2");
        mvprintw(7, 2, "  3 - Stage 3");
        mvprintw(8, 2, "  4 - Stage 4");
        refresh();
        while (true) {
            const int k = getch();
            if (k >= '1' && k <= '4') return k - '0';
        }
    }

    /*
     * [tickGate] 게이트 스폰 타이머를 처리한다.
     *
     * 동작 규칙:
     *   - 게이트가 비활성 상태이고, 마지막 스폰 후 GATE_SPAWN_MS(10초)가 지나면 새 게이트 생성
     *   - 게이트가 활성 상태라도 gateLock이 0이고 10초가 지나면 게이트를 재배치한다.
     *     (gateLock은 게이트 통과 직후 몸통이 완전히 빠져나갈 때까지 재스폰을 방지하는 카운터)
     *
     * 매개변수:
     *   now      - 현재 시각
     *   lastGate - 마지막 게이트 스폰(또는 통과) 시각 (갱신됨)
     *   gateLock - 뱀 몸통이 게이트를 완전히 빠져나갈 때까지의 남은 칸 수
     *   stage    - 현재 스테이지 (게이트 위치 선택에 사용)
     */
    void tickGate(const steady_clock::time_point& now,
                  steady_clock::time_point& lastGate,
                  int& gateLock, int stage) {
        if (!gateMgr.gatesActive) {
            // 게이트 없음 → 10초 후 최초 생성
            if (duration_cast<milliseconds>(now - lastGate).count() >= GATE_SPAWN_MS) {
                gateMgr.spawn(gmap, stage); lastGate = now;
            }
        } else if (gateLock == 0 &&
                   duration_cast<milliseconds>(now - lastGate).count() >= GATE_SPAWN_MS) {
            // 게이트 있음 + 뱀이 완전히 빠져나옴 + 10초 경과 → 재배치
            gateMgr.spawn(gmap, stage); lastGate = now;
        }
    }

    /*
     * [afterGateMove] 뱀이 게이트를 사용했는지 확인하고 gateLock을 갱신한다.
     *
     * - 이번 이동에서 게이트를 통과했으면: gateLock을 현재 뱀 길이로 설정하고
     *   lastGate를 현재 시각으로 갱신한다. (게이트가 몸통으로 덮여 있는 동안 재스폰 방지)
     * - 통과하지 않았으면: gateLock을 1씩 감소시켜 카운트다운한다.
     */
    void afterGateMove(int& gateLock, steady_clock::time_point& lastGate,
                       const steady_clock::time_point& now) {
        if (snake.lastMoveUsedGate) { gateLock = (int)snake.body.size(); lastGate = now; }
        else if (gateLock > 0) --gateLock;
    }

    // ──────────────────────────────────────────────────────────────────────────
    // 스테이지별 게임 루프
    // 공통 구조:
    //   1) reset(stage)으로 초기화
    //   2) 첫 프레임 렌더링
    //   3) while(true) 루프:
    //      a) 키 입력 처리 (handleInput → U턴 시 break)
    //      b) 아이템 만료 처리
    //      c) 게이트 타이머 처리
    //      d) 아이템 스폰 타이머 처리
    //      e) TICK_MS마다 뱀 이동 + 충돌 판정 + 렌더링 + 미션 체크
    //   4) 클리어 시 true, 사망 시 false 반환
    // ──────────────────────────────────────────────────────────────────────────

    /*
     * [runStage1] 스테이지 1 루프.
     * 고정 속도 150ms/틱, 클리어 조건: 최대 길이 10 + 먹이 5 + 독 2 + 게이트 1
     */
    bool runStage1() {
        reset(1);
        clear();
        renderer.drawMap(gmap);
        scoreboard.drawStage1(snake);
        refresh();

        auto lastTick = steady_clock::now();
        auto lastItem = steady_clock::now();
        auto lastGate = steady_clock::now();
        int  gateLock = 0;
        const int TICK_MS = 150;   // 뱀이 한 칸 이동하는 간격 (ms)

        while (true) {
            if (snake.handleInput()) break;   // U턴 입력 → 게임 오버

            const auto now = steady_clock::now();
            itemMgr.expire(gmap);             // 만료 아이템 제거
            tickGate(now, lastGate, gateLock, 1);

            // ITEM_INTERVAL_MS(3초)마다 아이템 스폰 시도
            if (duration_cast<milliseconds>(now - lastItem).count() >= ITEM_INTERVAL_MS) {
                itemMgr.spawn(gmap, 1); lastItem = now;
            }

            // TICK_MS마다 뱀 이동 및 화면 갱신
            if (duration_cast<milliseconds>(now - lastTick).count() >= TICK_MS) {
                if (!snake.move(gmap, itemMgr, gateMgr)) break;   // 충돌 → 사망
                afterGateMove(gateLock, lastGate, now);
                renderer.drawMap(gmap);
                scoreboard.drawStage1(snake);
                refresh();
                lastTick = now;

                // 미션 달성 체크
                if (Mission::checkStage1(snake.maxLength, snake.growthCount,
                                         snake.poisonCount, snake.gateCount))
                    return true;
            }
        }
        return false;
    }

    /*
     * [runStage2] 스테이지 2 루프.
     * 30초 생존 미션. 5초마다 틱 속도가 30ms씩 빨라지며, 최소 70ms까지 증가한다.
     *
     * speedLv: 속도 레벨 표시용 (1부터 시작, 스피드업 시마다 +1)
     * timeLeft: 남은 시간(초) = 30 - 경과 시간
     */
    bool runStage2() {
        reset(2);
        clear();

        const int SPEEDUP_MS  = 5000;  // 속도 증가 간격 (5초)
        const int BASE_TICK   = 150;   // 초기 틱 간격 (ms)
        const int TICK_DEC    = 30;    // 속도 증가 시 틱 감소량 (ms)
        const int MIN_TICK    = 70;    // 최소 틱 간격 (최대 속도)
        const int DURATION_MS = 30000; // 목표 생존 시간 (30초)

        int tickMs  = BASE_TICK;
        int speedLv = 1;

        auto lastTick    = steady_clock::now();
        auto lastItem    = steady_clock::now();
        auto lastGate    = steady_clock::now();
        auto stageStart  = steady_clock::now();   // 스테이지 시작 시각 (생존 시간 측정)
        auto lastSpeedUp = steady_clock::now();
        int  gateLock    = 0;

        renderer.drawMap(gmap);
        scoreboard.drawStage2(snake, 30, speedLv);
        refresh();

        while (true) {
            if (snake.handleInput()) break;

            const auto now       = steady_clock::now();
            const long elapsedMs = duration_cast<milliseconds>(now - stageStart).count();
            const int  timeLeft  = 30 - (int)(elapsedMs / 1000);   // 남은 시간 (초)

            itemMgr.expire(gmap);
            tickGate(now, lastGate, gateLock, 2);

            // 속도 증가 조건: 현재 속도가 최소보다 크고, 마지막 증가 후 5초 경과
            if (tickMs > MIN_TICK &&
                duration_cast<milliseconds>(now - lastSpeedUp).count() >= SPEEDUP_MS) {
                tickMs = max(MIN_TICK, tickMs - TICK_DEC);
                speedLv++; lastSpeedUp = now;
            }

            if (duration_cast<milliseconds>(now - lastItem).count() >= ITEM_INTERVAL_MS) {
                itemMgr.spawn(gmap, 2); lastItem = now;
            }

            if (duration_cast<milliseconds>(now - lastTick).count() >= tickMs) {
                if (!snake.move(gmap, itemMgr, gateMgr)) break;
                afterGateMove(gateLock, lastGate, now);
                renderer.drawMap(gmap);
                scoreboard.drawStage2(snake, max(0, timeLeft), speedLv);
                refresh();
                lastTick = now;
                if (Mission::checkStage2(elapsedMs)) return true;   // 30초 생존 달성
            }
        }
        return false;
    }

    /*
     * [runStage3] 스테이지 3 루프.
     * 먹이에 트랩이 동반되며, 트랩(8)을 밟으면 즉시 사망.
     * 고정 속도 150ms/틱, 클리어 조건: 먹이 7개 섭취
     */
    bool runStage3() {
        reset(3);
        clear();

        const int TICK_MS = 150;
        auto lastTick = steady_clock::now();
        auto lastItem = steady_clock::now();
        auto lastGate = steady_clock::now();
        int  gateLock = 0;

        renderer.drawMap(gmap);
        scoreboard.drawStage3(snake);
        refresh();

        while (true) {
            if (snake.handleInput()) break;

            const auto now = steady_clock::now();
            itemMgr.expire(gmap);
            tickGate(now, lastGate, gateLock, 3);

            if (duration_cast<milliseconds>(now - lastItem).count() >= ITEM_INTERVAL_MS) {
                itemMgr.spawn(gmap, 3); lastItem = now;
            }

            if (duration_cast<milliseconds>(now - lastTick).count() >= TICK_MS) {
                if (!snake.move(gmap, itemMgr, gateMgr)) break;
                afterGateMove(gateLock, lastGate, now);
                renderer.drawMap(gmap);
                scoreboard.drawStage3(snake);
                refresh();
                lastTick = now;
                if (Mission::checkStage3(snake.growthCount)) return true;
            }
        }
        return false;
    }

    /*
     * [runStage4] 스테이지 4 루프 (스테이지 1+2 종합).
     * 5초마다 속도 증가(10ms씩, 최소 70ms), 경과 시간 표시.
     * 클리어 조건: 길이 10 + 먹이 7 + 독 3 + 게이트 2 + 30초 생존
     */
    bool runStage4() {
        reset(4);
        clear();

        const int SPEEDUP_MS = 5000;
        const int BASE_TICK  = 150;
        const int TICK_DEC   = 10;    // 스테이지 4는 속도 증가 폭이 작음
        const int MIN_TICK   = 70;

        int tickMs  = BASE_TICK;
        int speedLv = 1;

        auto lastTick    = steady_clock::now();
        auto lastItem    = steady_clock::now();
        auto lastGate    = steady_clock::now();
        auto stageStart  = steady_clock::now();
        auto lastSpeedUp = steady_clock::now();
        int  gateLock    = 0;

        renderer.drawMap(gmap);
        scoreboard.drawStage4(snake, 0, speedLv);
        refresh();

        while (true) {
            if (snake.handleInput()) break;

            const auto now     = steady_clock::now();
            const int  elapsed = (int)(duration_cast<milliseconds>(now - stageStart).count() / 1000);

            itemMgr.expire(gmap);
            tickGate(now, lastGate, gateLock, 4);

            if (tickMs > MIN_TICK &&
                duration_cast<milliseconds>(now - lastSpeedUp).count() >= SPEEDUP_MS) {
                tickMs = max(MIN_TICK, tickMs - TICK_DEC);
                speedLv++; lastSpeedUp = now;
            }

            if (duration_cast<milliseconds>(now - lastItem).count() >= ITEM_INTERVAL_MS) {
                itemMgr.spawn(gmap, 4); lastItem = now;
            }

            if (duration_cast<milliseconds>(now - lastTick).count() >= tickMs) {
                if (!snake.move(gmap, itemMgr, gateMgr)) break;
                afterGateMove(gateLock, lastGate, now);
                renderer.drawMap(gmap);
                scoreboard.drawStage4(snake, elapsed, speedLv);
                refresh();
                lastTick = now;

                if (Mission::checkStage4(snake.maxLength, snake.growthCount,
                                         snake.poisonCount, snake.gateCount, elapsed))
                    return true;
            }
        }
        return false;
    }
};


// ══════════════════════════════════════════════════════════════════════════════
// main : ncurses 초기화 → 게임 실행 → ncurses 종료
// ══════════════════════════════════════════════════════════════════════════════
int main() {
    srand(time(nullptr));   // 아이템·게이트 위치 무작위화를 위한 랜덤 시드

    initscr();              // ncurses 초기화 (터미널을 raw 모드로 전환)
    noecho();               // 키 입력이 화면에 출력되지 않도록 설정
    curs_set(0);            // 커서 숨기기
    keypad(stdscr, TRUE);   // 방향키 등 특수키를 KEY_* 상수로 받기 위해 활성화
    timeout(-1);            // 처음에는 블로킹 입력 (스테이지 선택 화면)

    Game game;
    game.renderer.initColors();
    game.run();

    endwin();   // ncurses 종료, 터미널 원래 상태로 복원
    return 0;
}
