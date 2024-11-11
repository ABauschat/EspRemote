#ifndef SNAKEGAMESTATE_H
#define SNAKEGAMESTATE_H

#include "State.h"

namespace NuggetsInc {

class SnakeGameState : public AppState {
public:
    SnakeGameState();
    ~SnakeGameState();

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    void initGame();
    void updateSnake();
    void drawGame();
    void spawnApple();
    void gameOver();

    static const int SCREEN_WIDTH = 536;
    static const int SCREEN_HEIGHT = 220;
    static const int SNAKE_SIZE = 10;
    static const int SCORE_AREA_HEIGHT = 30; 
    static const int BOTTOM_MARGIN = 10; // Changed from 2 to 10
    static const int GAME_AREA_HEIGHT = SCREEN_HEIGHT - SCORE_AREA_HEIGHT - BOTTOM_MARGIN;
    static const int MAX_SNAKE_LENGTH = 1000;

    struct Point {
        int x;
        int y;
    };

    Point snake[MAX_SNAKE_LENGTH];
    int snakeLength;
    Point apple;
    Point prevApple;
    int snakeDirection; // 0=up, 1=right, 2=down, 3=left

    unsigned long lastUpdateTime;
    unsigned long updateInterval;

    int score;
    bool updateScore;

    // New member variables for optimized drawing
    Point prevTail;
    Point newHead;
};

} // namespace NuggetsInc

#endif // SNAKEGAMESTATE_H
