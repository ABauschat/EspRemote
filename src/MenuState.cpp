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
        

    // ESP-Connect
    // Remote Control
    // NFC Options
    // IR Options
    // APPS
    // Settings
    // Power

    menu[0] = "ESP-Connect";
    menu[1] = "Remote Control";
    menu[2] = "NFC Options";
    menu[3] = "IR Options";
    menu[4] = "Applications";
    menu[5] = "Power";
    menu[6] = "Settings";

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
            case EVENT_ACTION_ONE:
                executeSelection();
                break;
            case EVENT_ACTION_TWO:
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
        gfx->setCursor(10, i * 30);
        gfx->println(menu[i].c_str());
    }
}

void MenuState::executeSelection() {
    Application& app = Application::getInstance();
    Arduino_GFX* gfx = Device::getInstance().getDisplay();

    switch (menuIndex) {
    case 0: // ESP-Connect
            app.changeState(StateFactory::createState(ENTER_REMOTE_CONTROL_STATE));
            break;
        case 1: // Remote Control
            app.changeState(StateFactory::createState(IR_REMOTE_STATE));
            break;
        case 2: // NFC Options
            app.changeState(StateFactory::createState(NFC_OPTIONS_STATE));
            break;
        case 3: // IR Options
            app.changeState(StateFactory::createState(IR_OPTIONS_STATE));
            break;
        case 4: // Applications
            app.changeState(StateFactory::createState(APPLICATION_STATE));
            break;
        case 5: // Power
            app.changeState(StateFactory::createState(POWER_OPTIONS_STATE));
            break;
        case 6: // Settings
            app.changeState(StateFactory::createState(SETTINGS_STATE));
            break;
        default:
            break;
    }
}

} // namespace NuggetsInc
