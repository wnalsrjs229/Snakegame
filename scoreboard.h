#pragma once
#include "snake.h"

#define MISSION_S1_B    10
#define MISSION_S1_G    5
#define MISSION_S1_P    2
#define MISSION_S1_GATE 1
#define MISSION_G3      7
#define MISSION_S4_B    10
#define MISSION_S4_G    7
#define MISSION_S4_P    3
#define MISSION_S4_GATE 2
#define MISSION_S4_TIME 30

// ── 미션 클리어 조건 체크 ─────────────────────────────────────────────────────
class Mission {
public:
    static bool checkStage1(int maxLen, int growth, int poison, int gate);
    static bool checkStage2(long elapsedMs);
    static bool checkStage3(int growth);
    static bool checkStage4(int maxLen, int growth, int poison, int gate, int elapsed);
};

// ── 스코어보드 그리기 ─────────────────────────────────────────────────────────
class ScoreBoard {
public:
    void drawStage1(const Snake& snake);
    void drawStage2(const Snake& snake, int timeLeft, int speedLv);
    void drawStage3(const Snake& snake);
    void drawStage4(const Snake& snake, int elapsed, int speedLv);

private:
    void pCheck(bool done);
    void pHeader(int stageNum);
};
