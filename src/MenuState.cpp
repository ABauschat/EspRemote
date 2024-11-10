#include "MenuState.h"
#include "Device.h"
#include "Sounds.h"
#include "StateFactory.h"
#include "Application.h"

namespace NuggetsInc {

MenuState::MenuState() : menuIndex(0) {
    menu[0] = "Connect To Device";
    menu[1] = "Setup NFC Chip";
    menu[2] = "Clone NFC Chip";
    menu[3] = "Play Snake Game";
}

MenuState::~MenuState() {}

void MenuState::onEnter() {
    displayMenu();
}

void MenuState::onExit() {
    // Clean up if necessary
}

void MenuState::update() {
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    while (eventManager.getNextEvent(event)) {
        switch (event.type) {
            case EVENT_UP:
                menuIndex--;
                if (menuIndex < 0)
                    menuIndex = menuItems - 1;
                displayMenu();
                break;
            case EVENT_DOWN:
                menuIndex++;
                if (menuIndex >= menuItems)
                    menuIndex = 0;
                displayMenu();
                break;
            case EVENT_SELECT:
                executeSelection();
                break;
            case EVENT_BACK:
                // Do nothing in the main menu
                break;
            default:
                break;
        }
    }
}

void MenuState::displayMenu() {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);
    gfx->setTextSize(2);

    for (int i = 0; i < menuItems; i++) {
        if (i == menuIndex) {
            // Highlight the selected menu item
            gfx->setTextColor(RED);
        } else {
            gfx->setTextColor(WHITE);
        }
        gfx->setCursor(10, 30 + i * 30);
        gfx->println(menu[i]);
    }
}

void MenuState::executeSelection() {
    Application& app = Application::getInstance();
    Arduino_GFX* gfx = Device::getInstance().getDisplay();

    switch (menuIndex) {
        case 0: // Connect To Device
            gfx->fillScreen(BLACK);
            gfx->setTextColor(WHITE);
            gfx->setTextSize(2);
            gfx->setCursor(10, 100);
            gfx->println("Connecting...");
            delay(2000);
            displayMenu();
            break;
        case 1: // Setup NFC Chip
            gfx->fillScreen(BLACK);
            gfx->setTextColor(WHITE);
            gfx->setTextSize(2);
            gfx->setCursor(10, 100);
            gfx->println("Setting up NFC...");
            delay(2000);
            displayMenu();
            break;
        case 2: // Clone NFC Chip
            gfx->fillScreen(BLACK);
            gfx->setTextColor(WHITE);
            gfx->setTextSize(2);
            gfx->setCursor(10, 100);
            gfx->println("Cloning NFC...");
            delay(2000);
            displayMenu();
            break;
        case 3: // Play Snake Game
            app.changeState(StateFactory::createState(SNAKE_GAME_STATE));
            break;
        default:
            break;
    }
}

} // namespace NuggetsInc
