// IRRemoteState.cpp
#include "IRRemoteState.h"
#include "DisplayUtils.h"
#include "Device.h"
#include <Arduino.h>

namespace NuggetsInc
{
    IRRemoteState::IRRemoteState()
        : displayUtils(nullptr)
    {}

    IRRemoteState::~IRRemoteState()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
    }

    void IRRemoteState::onEnter()
    {
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
        displayUtils->newTerminalDisplay("Adafruit IR Transceiver");

        BeginIrSender();

        String result = LoadIRData(buttonIRData);
        displayUtils->addToTerminalDisplay(result);
    }

    void IRRemoteState::onExit()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
    }

    void IRRemoteState::update()
    {
        EventManager &eventManager = EventManager::getInstance();
        Event event;

        while (eventManager.getNextEvent(event))
        {
            ButtonType button = mapEventTypeToButtonType(event.type);
            if (button != BUTTON_COUNT)
            {
                unsigned long currentTime = millis();

                if (buttonIRData[button].isValid)
                {
                    displayUtils->addToTerminalDisplay("Sending stored IR data for button...");

                    String result = SendIRData(buttonIRData, button);

                    displayUtils->addToTerminalDisplay(result);
                }
                else
                {
                    displayUtils->addToTerminalDisplay("No IR data stored for button.");
                }
            }
        }
    }

} // namespace NuggetsInc