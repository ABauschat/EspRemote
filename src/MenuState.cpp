// MenuState.cpp
#include "MenuState.h"
#include "Device.h"
#include "Sounds.h"
#include "StateFactory.h"
#include "Application.h"
#include "EventManager.h"
#include "Colors.h"

namespace NuggetsInc {

MenuState::MenuState()
    : menuIndex(0),
      displayUtils(nullptr) {
    menu[0] = "Connect To Device";
    menu[1] = "Setup NFC Chip";
    menu[2] = "Clone NFC Chip";
    menu[3] = "Play Snake Game";

    // Initialize DisplayUtils
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
}

MenuState::~MenuState() {
    delete displayUtils;
}

void MenuState::onEnter() {
    displayMenu();
}

void MenuState::onExit() {

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
                break;
            default:
                break;
        }
    }
}

void MenuState::displayMenu() {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextSize(2);

    for (int i = 0; i < menuItems; i++) {
        if (i == menuIndex) {
            gfx->setTextColor(COLOR_ORANGE);
        }
        else {
            gfx->setTextColor(COLOR_WHITE);
        }
        gfx->setCursor(10, 30 + i * 30);
        gfx->println(menu[i].c_str());
    }
}

void MenuState::executeSelection() {
    Application& app = Application::getInstance();
    Arduino_GFX* gfx = Device::getInstance().getDisplay();

    switch (menuIndex) {
        case 0: 
            app.changeState(StateFactory::createState(ENTER_REMOTE_CONTROL_STATE));
            break;
        case 1: 
            app.changeState(StateFactory::createState(SETUP_NFC_DEVICE_STATE));
            break;
        case 2: // Clone NFC Chip
            app.changeState(StateFactory::createState(CLONE_NFC_STATE));
            break;
        case 3: // Play Snake Game
            app.changeState(StateFactory::createState(SNAKE_GAME_STATE));
            break;
        default:
            break;
    }
}

} // namespace NuggetsInc
