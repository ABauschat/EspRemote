#include "RemoteControlState.h"
#include "StateFactory.h"
#include "Application.h"
#include "DisplayUtils.h"
#include "Config.h"

namespace NuggetsInc
{

    RemoteControlState::RemoteControlState()
        : nfcLogic(new NFCLogic(PN532_IRQ, PN532_RESET)) {}

    RemoteControlState::~RemoteControlState()
    {
        delete nfcLogic;
    }

    void RemoteControlState::onEnter()
    {
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
        displayRemoteControlInterface();
    }

    void RemoteControlState::onExit()
    {
    }

    void RemoteControlState::update()
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
            else
            {
                handleInput(event.type);
            }
        }
    }

    void RemoteControlState::handleInput(EventType eventType)
    {
        switch (eventType)
        {
        case EVENT_UP:
            // Send command to move up
            break;
        case EVENT_DOWN:
            // Send command to move down
            break;
        case EVENT_LEFT:
            // Send command to move left
            break;
        case EVENT_RIGHT:
            // Send command to move right
            break;
        case EVENT_SELECT:
            // Perform selection or action
            break;
        default:
            break;
        }
    }

    void RemoteControlState::displayRemoteControlInterface()
    {
        // Display remote control interface
        displayUtils->newTerminalDisplay("Remote Control Interface");
        displayUtils->addToTerminalDisplay("Use arrow keys to navigate");
        displayUtils->addToTerminalDisplay("Press SELECT to perform action");
    }

} // namespace NuggetsInc
