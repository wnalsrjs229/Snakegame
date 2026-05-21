#include "scoreboard.h"

// ── Mission ───────────────────────────────────────────────────────────────────

bool Mission::checkStage1(int maxLen, int growth, int poison, int gate) {
    return maxLen  >= MISSION_S1_B    &&
           growth  >= MISSION_S1_G    &&
           poison  >= MISSION_S1_P    &&
           gate    >= MISSION_S1_GATE;
}

bool Mission::checkStage2(long elapsedMs) {
    return elapsedMs >= 30000;
}

bool Mission::checkStage3(int growth) {
    return growth >= MISSION_G3;
}

bool Mission::checkStage4(int maxLen, int growth, int poison, int gate, int elapsed) {
    return maxLen  >= MISSION_S4_B    &&
           growth  >= MISSION_S4_G    &&
           poison  >= MISSION_S4_P    &&
           gate    >= MISSION_S4_GATE &&
           elapsed >= MISSION_S4_TIME;
}

// ── ScoreBoard ────────────────────────────────────────────────────────────────

void ScoreBoard::drawStage1(const Snake& snake) {
    pHeader(1);
    int row = IPY + 2;

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

void ScoreBoard::drawStage2(const Snake& snake, int timeLeft, int speedLv) {
    pHeader(2);
    int row = IPY + 2;

    mvprintw(row,   IPX, "Length:  %d",    (int)snake.body.size());
    mvprintw(row+1, IPX, "Time:    %ds",   timeLeft);
    mvprintw(row+2, IPX, "Speed:   Lv.%d", speedLv);

    mvprintw(row,   IPMID, "Survive 30s");
    mvprintw(row+1, IPMID, "Time  ");
    pCheck(timeLeft <= 0);
}

void ScoreBoard::drawStage3(const Snake& snake) {
    pHeader(3);
    int row = IPY + 2;

    attron(COLOR_PAIR(PAIR_TEXT_GOOD));
    mvprintw(row,   IPX, "+: %d", snake.growthCount);
    attroff(COLOR_PAIR(PAIR_TEXT_GOOD));
    mvprintw(row,   IPMID, "Eat good items: %d", MISSION_G3);
    mvprintw(row+1, IPMID, "+  ");
    pCheck(snake.growthCount >= MISSION_G3);
}

void ScoreBoard::drawStage4(const Snake& snake, int elapsed, int speedLv) {
    pHeader(4);
    int row = IPY + 2;

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

void ScoreBoard::pHeader(int stageNum) {
    for (int y = IPY; y < IPY + IPH; y++)
        mvprintw(y, IPX, "%-28s", "");
    attron(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
    mvprintw(IPY, IPX,   "Stage %d", stageNum);
    mvprintw(IPY, IPMID, "Mission");
    attroff(COLOR_PAIR(PAIR_TITLE) | A_BOLD);
}
