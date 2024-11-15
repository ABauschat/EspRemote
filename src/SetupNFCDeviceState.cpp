#include "SetupNFCDeviceState.h"
#include "StateFactory.h"
#include "Application.h"
#include "DisplayUtils.h"
#include "Colors.h"
#include "Config.h"

namespace NuggetsInc
{

    SetupNFCDeviceState::SetupNFCDeviceState()
        : nfcLogic(new NFCLogic(PN532_IRQ, PN532_RESET)) {}

    SetupNFCDeviceState::~SetupNFCDeviceState()
    {
        delete nfcLogic;
    }

    void SetupNFCDeviceState::onEnter()
    {
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
        displaySetupInstructions();
    }

    void SetupNFCDeviceState::onExit()
    {
    }

    void SetupNFCDeviceState::update()
    {
        EventManager &eventManager = EventManager::getInstance();
        Event event;

        while (eventManager.getNextEvent(event))
        {
            if (event.type == EVENT_BACK)
            {
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            }
            else if (event.type == EVENT_SELECT)
            {
            }
        }
    }

    void SetupNFCDeviceState::displaySetupInstructions()
    {
        displayUtils->newTerminalDisplay("Place NFC Device on Reader");
        displayUtils->addToTerminalDisplay("Press SELECT to Configure");
    }

} // namespace NuggetsInc
