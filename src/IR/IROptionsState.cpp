#include "IROptionsState.h"
#include "Device.h"
#include "StateFactory.h"
#include "Application.h"
#include "EventManager.h"
#include "Colors.h"

namespace NuggetsInc {

IROptionsState::IROptionsState()
    : menuIndex(0),
      displayUtils(nullptr) {
    
    // Define menu options
    menu[0] = "Setup New Remote";

    // Initialize DisplayUtils
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
}

IROptionsState::~IROptionsState() {
    delete displayUtils;
}

void IROptionsState::onEnter() {
    displayMenu();
}

void IROptionsState::onExit() {
    // Clean up or reset if necessary
}

void IROptionsState::update() {
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    while (eventManager.getNextEvent(event)) {
        switch (event.type) {
            case EVENT_UP:
                menuIndex--;
                if (menuIndex < 0)
                    menuIndex = menuItems - 1; // Wrap to last menu item
                displayMenu();
                break;
            case EVENT_DOWN:
                menuIndex++;
                if (menuIndex >= menuItems)
                    menuIndex = 0; // Wrap to first menu item
                displayMenu();
                break;
            case EVENT_ACTION_ONE:
                executeSelection();
                break;
            case EVENT_ACTION_TWO:
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                break;
            default:
                break;
        }
    }
}

void IROptionsState::displayMenu() {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextSize(2);

    for (int i = 0; i < menuItems; i++) {
        if (i == menuIndex) {
            gfx->setTextColor(COLOR_ORANGE); // Highlight selected item
        } else {
            gfx->setTextColor(COLOR_WHITE);
        }
        gfx->setCursor(10, i * 30);
        gfx->println(menu[i].c_str());
    }
}

void IROptionsState::executeSelection() {
    Application& app = Application::getInstance();

    switch (menuIndex) {
        case 0: // Remote Browser
            app.changeState(StateFactory::createState(SETUP_NEW_REMOTE_STATE));
            break;
    }
}

} // namespace NuggetsInc
