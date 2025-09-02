#include "NFCOptionsState.h"
#include "Device.h"
#include "StateFactory.h"
#include "Application.h"
#include "EventManager.h"
#include "Colors.h"

namespace NuggetsInc {

NFCOptionsState::NFCOptionsState()
    : menuIndex(0),
      displayUtils(nullptr) {
    
    // Define menu options
    menu[0] = "Setup NFC Device";
    menu[1] = "Clone NFC Chip";

    // Initialize DisplayUtils
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
}

NFCOptionsState::~NFCOptionsState() {
    delete displayUtils;
}

void NFCOptionsState::onEnter() {
    displayMenu();
}

void NFCOptionsState::onExit() {
    // Clean up or reset if necessary
}

void NFCOptionsState::update() {
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

void NFCOptionsState::displayMenu() {
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

void NFCOptionsState::executeSelection() {
    Application& app = Application::getInstance();

    switch (menuIndex) {
        case 0: // Setup NFC Device
            app.changeState(StateFactory::createState(SETUP_NFC_DEVICE_STATE));
            break;
        case 1: // Clone NFC Chip
            app.changeState(StateFactory::createState(CLONE_NFC_STATE));
            break;
        default:
            // Handle unexpected cases gracefully
            break;
    }
}

} // namespace NuggetsInc
