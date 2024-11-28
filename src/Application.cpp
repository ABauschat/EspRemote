#include "Application.h"
#include "StateFactory.h"
#include "Device.h"
#include "Sounds.h"

namespace NuggetsInc {

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

Application::Application() : currentState(nullptr) {}

void Application::init() {
    Device::getInstance().init();

    // Clear the screen and set text properties
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);
    gfx->setCursor(60, 40);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->println("Welcome To Nuggets Inc!");

    // Play the startup melody
    //Sounds::getInstance().playMelody();
    //delay(1000); // Delay to allow the melody to play

    // Start with the menu state
    changeState(StateFactory::createState(MENU_STATE));
}

void Application::run() {
    Device::getInstance().update(); // Update device to read inputs
    if (currentState) {
        currentState->update();
    }
}

void Application::changeState(AppState* newState) {
    if (currentState) {
        currentState->onExit();
        delete currentState;
    }
    currentState = newState;
    if (currentState) {
        currentState->onEnter();
    }
}

} // namespace NuggetsInc
