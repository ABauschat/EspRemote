#include "SnakeGameState.h"
#include "Device.h"
#include "Sounds.h"
#include "StateFactory.h"
#include "Application.h"

namespace NuggetsInc {

SnakeGameState::SnakeGameState()
    : snakeLength(5), snakeDirection(1), lastUpdateTime(0),
      updateInterval(200), score(0) {}

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

    spawnApple(); // Spawn the first apple
    score = 0;
    lastUpdateTime = millis();

    // Clear the screen
    Device::getInstance().getDisplay()->fillScreen(BLACK);
}

void SnakeGameState::spawnApple() {
    bool validPosition = false;
    while (!validPosition) {
        // Generate random positions within the game area
        int maxX = SCREEN_WIDTH / SNAKE_SIZE;
        int minY = SCORE_AREA_HEIGHT / SNAKE_SIZE;
        int maxY = SCREEN_HEIGHT / SNAKE_SIZE;

        apple.x = random(0, maxX) * SNAKE_SIZE;
        apple.y = random(minY, maxY) * SNAKE_SIZE;

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
    // Store the previous tail position
    Point prevTail = snake[snakeLength - 1];

    // Move each segment to the position of the previous one
    for (int i = snakeLength - 1; i > 0; i--) {
        snake[i] = snake[i - 1];
    }

    // Update head position based on direction
    switch (snakeDirection) {
        case 0: // Up
            snake[0].y -= SNAKE_SIZE;
            break;
        case 1: // Right
            snake[0].x += SNAKE_SIZE;
            break;
        case 2: // Down
            snake[0].y += SNAKE_SIZE;
            break;
        case 3: // Left
            snake[0].x -= SNAKE_SIZE;
            break;
    }

    // Boundary checks for horizontal movement with wrapping
    if (snake[0].x < 0)
        snake[0].x = SCREEN_WIDTH - SNAKE_SIZE;
    if (snake[0].x >= SCREEN_WIDTH)
        snake[0].x = 0;

    // Boundary checks for vertical movement without wrapping
    if (snake[0].y < SCORE_AREA_HEIGHT) {
        snake[0].y = SCORE_AREA_HEIGHT;
        // Optional feedback
        Sounds::getInstance().playTone(500, 100); // Example feedback
    }
    if (snake[0].y >= SCREEN_HEIGHT) {
        snake[0].y = SCREEN_HEIGHT - SNAKE_SIZE;
        // Optional feedback
        Sounds::getInstance().playTone(500, 100); // Example feedback
    }

    // Check for collisions with self
    for (int i = 1; i < snakeLength; i++) {
        if (snake[0].x == snake[i].x && snake[0].y == snake[i].y) {
            gameOver();
            return;
        }
    }

    // Check for apple collision
    if (snake[0].x == apple.x && snake[0].y == apple.y) {
        score++;
        if (snakeLength < MAX_SNAKE_LENGTH) {
            // Add new segment at the tail
            snake[snakeLength] = prevTail;
            snakeLength++;
        }
        spawnApple();
        Sounds::getInstance().playTone(1000, 100);
    }
}

void SnakeGameState::drawGame() {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);
    
    // Draw the horizontal line separating score and game area
    gfx->drawFastHLine(0, SCORE_AREA_HEIGHT, SCREEN_WIDTH, WHITE);

    // Draw apple only within the game area
    if (apple.y >= SCORE_AREA_HEIGHT && apple.y < SCREEN_HEIGHT) {
        gfx->fillRect(apple.x, apple.y, SNAKE_SIZE, SNAKE_SIZE, RED);
    }

    // Draw snake segments only within the game area
    for (int i = 0; i < snakeLength; i++) {
        if (snake[i].y >= SCORE_AREA_HEIGHT && snake[i].y < SCREEN_HEIGHT) {
            gfx->fillRect(snake[i].x, snake[i].y, SNAKE_SIZE, SNAKE_SIZE, GREEN);
        }
    }

    // Draw score
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(0, 5); // Adjust y-position to fit within score area
    gfx->print("Score: ");
    gfx->println(score);
}

void SnakeGameState::gameOver() {
    EventManager::getInstance().clearEvents();  

    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);
    gfx->setTextColor(RED);
    gfx->setTextSize(3);
    gfx->setCursor(100, SCREEN_HEIGHT / 2 - 30);
    gfx->println("Game Over");
    gfx->setTextSize(2);
    gfx->setCursor(100, SCREEN_HEIGHT / 2);
    gfx->print("Score: ");
    gfx->println(score);
    gfx->setCursor(80, SCREEN_HEIGHT / 2 + 30);
    gfx->println("Returning to Menu");
    delay(3000);

    // Transition to Clear State first, then to MenuState
    Application::getInstance().changeState(StateFactory::createClearStateWithNext(MENU_STATE));

}

} // namespace NuggetsInc
