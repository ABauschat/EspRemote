#include "SetupNFCDeviceState.h"
#include "StateFactory.h"
#include "Application.h"
#include "DisplayUtils.h"
#include "Colors.h"
#include "Config.h"
#include <Device.h>

namespace NuggetsInc {

SetupNFCDeviceState::SetupNFCDeviceState()
    : nfcLogic(new NFCLogic(PN532_IRQ, PN532_RESET)),
      deviceConfigured(false),
      displayUtils(displayUtils) {}

SetupNFCDeviceState::~SetupNFCDeviceState() {
    delete nfcLogic;
}

void SetupNFCDeviceState::onEnter() {
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
    displaySetupInstructions();
}

void SetupNFCDeviceState::onExit() {
    // Cleanup if necessary
}

void SetupNFCDeviceState::update() {
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    while (eventManager.getNextEvent(event)) {
        if (event.type == EVENT_BACK) {
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            return;
        } else if (event.type == EVENT_SELECT) {
          
        }
    }
}

void SetupNFCDeviceState::displaySetupInstructions() {
    displayUtils->newTerminalDisplay("Place NFC Device on Reader");
    displayUtils->addToTerminalDisplay("Press SELECT to Configure");
}

// Remaining methods unchanged

} // namespace NuggetsInc
