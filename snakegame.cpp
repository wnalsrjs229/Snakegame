/*
 * Snake Game
 *
 * Stage 1: length 10, good 5, bad 2, gate 1
 * Stage 2: Survive 30 seconds — speed increases every 5 seconds
 * Stage 3: Eat 7 good items — each good item has an adjacent trap
 *          Map: horizontal wall through center (gap at center)
 * Stage 4: Combined — length 10, good 7, bad 3, gate 2, survive 30s
 *          Map: cross in center
 *
 * Controls:
 *   Arrow keys         — move snake
 *   Opposite direction — instant Game Over
 *
 * Compile:
 *   g++ -o snakegame snakegame.cpp map.cpp gate.cpp item.cpp snake.cpp mission.cpp scoreboard.cpp -lncurses
 *
 * Run:
 *   ./snakegame
 */

#include "scoreboard.h"

// ─────────────────────────────────────────
// Renderer  (map drawing + game messages)
// ─────────────────────────────────────────
class Renderer {
public:
    void initColors() {
        start_color();
        init_pair(PAIR_WALL,       COLOR_WHITE,   COLOR_WHITE);
        init_pair(PAIR_BODY,       COLOR_GREEN,   COLOR_GREEN);
        init_pair(PAIR_HEAD,       COLOR_GREEN,   COLOR_GREEN);
        init_pair(PAIR_GOOD,       COLOR_CYAN,    COLOR_CYAN);
        init_pair(PAIR_BAD,        COLOR_RED,     COLOR_RED);
        init_pair(PAIR_GATE,       COLOR_MAGENTA, COLOR_MAGENTA);
        init_pair(PAIR_TITLE,      COLOR_YELLOW,  COLOR_BLACK);
        init_pair(PAIR_DONE,       COLOR_GREEN,   COLOR_BLACK);
        init_pair(PAIR_TEXT_GOOD,  COLOR_CYAN,    COLOR_BLACK);
        init_pair(PAIR_TEXT_BAD,   COLOR_RED,     COLOR_BLACK);
        init_pair(PAIR_TEXT_GATE,  COLOR_MAGENTA, COLOR_BLACK);
        init_pair(PAIR_TRAP,       COLOR_YELLOW,  COLOR_YELLOW);
        init_pair(PAIR_BORDER,     COLOR_CYAN,    COLOR_BLACK);
        init_pair(PAIR_TEXT_WHITE, COLOR_WHITE,   COLOR_BLACK);
        init_pair(PAIR_TEXT_TRAP,  COLOR_YELLOW,  COLOR_BLACK);
    }

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
                    mvprintw(y, x*2, "  ");
                    break;
                }
            }
        }
    }

    void showGameOver() {
        attron(COLOR_PAIR(PAIR_TEXT_BAD) | A_BOLD);
        mvprintw(MAP_SIZE + 1, 0, "Game Over! Press any key...");
        attroff(COLOR_PAIR(PAIR_TEXT_BAD) | A_BOLD);
        refresh();
        timeout(-1);
        getch();
    }

    void showStageClear(int stage) {
        attron(COLOR_PAIR(PAIR_DONE) | A_BOLD);
        if (stage == 4)
            mvprintw(MAP_SIZE + 1, 0, "Game Clear! Press any key...");
        else
            mvprintw(MAP_SIZE + 1, 0, "Stage %d Clear! Press any key for Stage %d...", stage, stage + 1);
        attroff(COLOR_PAIR(PAIR_DONE) | A_BOLD);
        refresh();
        timeout(-1); getch(); timeout(0);
    }
};

// ─────────────────────────────────────────
// Game
// ─────────────────────────────────────────
class Game {
public:
    GameMap     gmap;
    Snake       snake;
    ItemManager itemMgr;
    GateManager gateMgr;
    Renderer    renderer;
    ScoreBoard  scoreboard;

    void run() {
        int startStage = selectStage();
        timeout(0);

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
    void reset(int stage) {
        itemMgr.clear();
        gateMgr.clear();
        gmap.init(stage);
        snake.reset(gmap, stage);
    }

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
            int k = getch();
            if (k >= '1' && k <= '4') return k - '0';
        }
    }

    void tickGate(const steady_clock::time_point& now,
                  steady_clock::time_point& lastGate,
                  int& gateLock, int stage) {
        if (!gateMgr.gatesActive) {
            if (duration_cast<milliseconds>(now - lastGate).count() >= GATE_SPAWN_MS) {
                gateMgr.spawn(gmap, stage); lastGate = now;
            }
        } else if (gateLock == 0 &&
                   duration_cast<milliseconds>(now - lastGate).count() >= GATE_SPAWN_MS) {
            gateMgr.spawn(gmap, stage); lastGate = now;
        }
    }

    void afterGateMove(int& gateLock, steady_clock::time_point& lastGate,
                       const steady_clock::time_point& now) {
        if (snake.lastMoveUsedGate) { gateLock = (int)snake.body.size(); lastGate = now; }
        else if (gateLock > 0) --gateLock;
    }

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
        const int TICK_MS = 150;

        while (true) {
            if (snake.handleInput()) break;

            auto now = steady_clock::now();
            itemMgr.expire(gmap);
            tickGate(now, lastGate, gateLock, 1);

            if (duration_cast<milliseconds>(now - lastItem).count() >= ITEM_INTERVAL_MS) {
                itemMgr.spawn(gmap, 1); lastItem = now;
            }

            if (duration_cast<milliseconds>(now - lastTick).count() >= TICK_MS) {
                if (!snake.move(gmap, itemMgr, gateMgr)) break;
                afterGateMove(gateLock, lastGate, now);
                renderer.drawMap(gmap);
                scoreboard.drawStage1(snake);
                refresh();
                lastTick = now;

                if (Mission::checkStage1(snake.maxLength, snake.growthCount,
                                         snake.poisonCount, snake.gateCount))
                    return true;
            }
        }
        return false;
    }

    bool runStage2() {
        reset(2);
        clear();

        const int SPEEDUP_MS  = 5000;
        const int BASE_TICK   = 150;
        const int TICK_DEC    = 30;
        const int MIN_TICK    = 70;
        const int DURATION_MS = 30000;

        int tickMs  = BASE_TICK;
        int speedLv = 1;

        auto lastTick    = steady_clock::now();
        auto lastItem    = steady_clock::now();
        auto lastGate    = steady_clock::now();
        auto stageStart  = steady_clock::now();
        auto lastSpeedUp = steady_clock::now();
        int  gateLock    = 0;

        renderer.drawMap(gmap);
        scoreboard.drawStage2(snake, 30, speedLv);
        refresh();

        while (true) {
            if (snake.handleInput()) break;

            auto now       = steady_clock::now();
            long elapsedMs = duration_cast<milliseconds>(now - stageStart).count();
            int  timeLeft  = 30 - (int)(elapsedMs / 1000);

            itemMgr.expire(gmap);
            tickGate(now, lastGate, gateLock, 2);

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
                if (Mission::checkStage2(elapsedMs)) return true;
            }
        }
        return false;
    }

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

            auto now = steady_clock::now();
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

    bool runStage4() {
        reset(4);
        clear();

        const int SPEEDUP_MS = 5000;
        const int BASE_TICK  = 150;
        const int TICK_DEC   = 10;
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

            auto now     = steady_clock::now();
            int  elapsed = (int)(duration_cast<milliseconds>(now - stageStart).count() / 1000);

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

// ─────────────────────────────────────────
// main
// ─────────────────────────────────────────
int main() {
    srand(time(nullptr));

    initscr();
    noecho();
    curs_set(0);
    keypad(stdscr, TRUE);
    timeout(-1);

    Game game;
    game.renderer.initColors();
    game.run();

    endwin();
    return 0;
}
