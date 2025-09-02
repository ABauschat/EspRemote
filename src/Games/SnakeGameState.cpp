#include "SnakeGameState.h"
#include "Device.h"
#include "Sounds.h"
#include "StateFactory.h"
#include "Application.h"
#include "Haptics.h"

namespace NuggetsInc {

SnakeGameState::SnakeGameState()
    : snakeLength(5), snakeDirection(1), lastUpdateTime(0),
      updateInterval(200), score(0), updateScore(true) {
    prevApple.x = -1;
    prevApple.y = -1;
}

SnakeGameState::~SnakeGameState() {}

void SnakeGameState::onEnter() {
    initGame();
}

void SnakeGameState::onExit() {
}

void SnakeGameState::update() {
    Device::getInstance().update(); // Update device to read inputs
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    // Handle events
    while (eventManager.getNextEvent(event)) {
        switch (event.type) {
            case EVENT_UP:
                if (snakeDirection != 2)
                    snakeDirection = 0;
                break;
            case EVENT_DOWN:
                if (snakeDirection != 0)
                    snakeDirection = 2;
                break;
            case EVENT_LEFT:
                if (snakeDirection != 1)
                    snakeDirection = 3;
                break;
            case EVENT_RIGHT:
                if (snakeDirection != 3)
                    snakeDirection = 1;
                break;
            case EVENT_ACTION_TWO:
            case EVENT_BACK:
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            default:
                break;
        }
    }

    unsigned long currentTime = millis();
    if (currentTime - lastUpdateTime > updateInterval) {
        lastUpdateTime = currentTime;
        updateSnake();
        drawGame();
    }
}

void SnakeGameState::initGame() {
    // Calculate the center position within the game area
    int startX = (SCREEN_WIDTH / SNAKE_SIZE / 2) * SNAKE_SIZE;
    int startY = SCORE_AREA_HEIGHT + (GAME_AREA_HEIGHT / SNAKE_SIZE / 2) * SNAKE_SIZE;

    // Initialize snake segments
    for (int i = 0; i < snakeLength; i++) {
        snake[i].x = startX - i * SNAKE_SIZE;
        snake[i].y = startY;
    }

    prevTail = snake[snakeLength - 1]; // Initialize previous tail
    newHead = snake[0]; // Initialize new head

    spawnApple(); // Spawn the first apple
    score = 0;
    updateScore = true; // Ensure the score is drawn initially
    lastUpdateTime = millis();

    // Initialize the display
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);

    // **Draw the score area separator line one pixel above**
    gfx->drawFastHLine(0, SCORE_AREA_HEIGHT - 1, SCREEN_WIDTH, WHITE);

    // Draw the bottom margin in white
    gfx->fillRect(0, SCREEN_HEIGHT - BOTTOM_MARGIN, SCREEN_WIDTH, BOTTOM_MARGIN, WHITE);

    // Draw initial snake
    for (int i = 0; i < snakeLength; i++) {
        gfx->fillRect(snake[i].x, snake[i].y, SNAKE_SIZE, SNAKE_SIZE, GREEN);
    }

    // Draw initial apple
    gfx->fillRect(apple.x, apple.y, SNAKE_SIZE, SNAKE_SIZE, RED);
}

void SnakeGameState::spawnApple() {
    bool validPosition = false;
    prevApple = apple; // Store previous apple position
    while (!validPosition) {
        // Adjusted maxX and maxY to prevent overflow
        int maxX = (SCREEN_WIDTH - SNAKE_SIZE) / SNAKE_SIZE; //52 for SCREEN_WIDTH=536
        int minY = SCORE_AREA_HEIGHT / SNAKE_SIZE; //3
        int maxY = (SCREEN_HEIGHT - BOTTOM_MARGIN - SNAKE_SIZE) / SNAKE_SIZE; //20

        apple.x = random(0, maxX + 1) * SNAKE_SIZE; // 0 to 520
        apple.y = random(minY, maxY + 1) * SNAKE_SIZE; // 30 to 200

        validPosition = true;

        // Ensure apple doesn't spawn on the snake
        for (int i = 0; i < snakeLength; i++) {
            if (snake[i].x == apple.x && snake[i].y == apple.y) {
                validPosition = false;
                break;
            }
        }
    }
}

void SnakeGameState::updateSnake() {
    // Compute the intended new head position
    Point intendedHead = snake[0];
    switch (snakeDirection) {
        case 0: // Up
            intendedHead.y -= SNAKE_SIZE;
            break;
        case 1: // Right
            intendedHead.x += SNAKE_SIZE;
            break;
        case 2: // Down
            intendedHead.y += SNAKE_SIZE;
            break;
        case 3: // Left
            intendedHead.x -= SNAKE_SIZE;
            break;
    }

    // Boundary checks before moving
    if (intendedHead.x < 0 || (intendedHead.x + SNAKE_SIZE) > SCREEN_WIDTH ||
        intendedHead.y < SCORE_AREA_HEIGHT || (intendedHead.y + SNAKE_SIZE) > (SCREEN_HEIGHT - BOTTOM_MARGIN)) {
        gameOver();
        return;
    }

    // Check for collision with self
    for (int i = 0; i < snakeLength; i++) {
        if (intendedHead.x == snake[i].x && intendedHead.y == snake[i].y) {
            gameOver();
            return;
        }
    }

    // Check for apple collision
    bool appleEaten = false;
    if (intendedHead.x == apple.x && intendedHead.y == apple.y) {
        appleEaten = true;
        score++;
        if (snakeLength < MAX_SNAKE_LENGTH) {
            snake[snakeLength] = prevTail;
            snakeLength++;
        }
        spawnApple();
        //Sounds::getInstance().playTone(1000, 100);
        Haptics::getInstance().singleVibration();
        updateScore = true;
    }

    // Move the snake
    prevTail = snake[snakeLength - 1];
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }
    snake[0] = intendedHead;
    newHead = snake[0];
}

void SnakeGameState::drawGame() {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();

    // Erase the previous tail segment
    gfx->fillRect(prevTail.x, prevTail.y, SNAKE_SIZE, SNAKE_SIZE, BLACK);

    // Draw the new head segment
    gfx->fillRect(newHead.x, newHead.y, SNAKE_SIZE, SNAKE_SIZE, GREEN);

    // Erase previous apple if it has moved
    if (prevApple.x != apple.x || prevApple.y != apple.y) {
        // Draw the new apple
        gfx->fillRect(apple.x, apple.y, SNAKE_SIZE, SNAKE_SIZE, RED);
    }

    // Draw score only if it has been updated
    if (updateScore) {
        // Clear the previous score by filling the score area background
        gfx->fillRect(0, 0, SCREEN_WIDTH, SCORE_AREA_HEIGHT, WHITE);

        // Draw the updated score
        gfx->setTextColor(BLACK);
        gfx->setTextSize(2);
        gfx->setCursor(0, 5); // Adjust y-position to fit within score area
        gfx->print("Score: ");
        gfx->println(score);

        updateScore = false; // Reset the flag
    }
}

void SnakeGameState::gameOver() {
    EventManager::getInstance().clearEvents();  

    Haptics::getInstance().doubleVibration();

    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);
    gfx->setTextColor(RED);
    gfx->setTextSize(3);
    gfx->setCursor(100, (SCREEN_HEIGHT / 2) - 30);
    gfx->println("Game Over");
    gfx->setTextSize(2);
    gfx->setCursor(100, SCREEN_HEIGHT / 2);
    gfx->print("Score: ");
    gfx->println(score);
    gfx->setCursor(80, (SCREEN_HEIGHT / 2) + 30);
    gfx->println("Returning to Menu");
    delay(3000);

    // Transition to MenuState
    Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
}

} // namespace NuggetsInc
