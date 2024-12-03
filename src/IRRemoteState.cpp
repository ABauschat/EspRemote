#include "IRRemoteState.h"
#include "DisplayUtils.h"
#include <IRremote.hpp>
#include "Device.h"
#include "EventManager.h"

#define IR_RECEIVE_PIN 14
#define IR_SEND_PIN 15

namespace NuggetsInc
{

    IRRemoteState::IRRemoteState()
        : displayUtils(nullptr), recordingButton(BUTTON_COUNT)
    {
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            buttonIRData[i].isValid = false;
            buttonIRData[i].rawDataLength = 0;
        }
    }

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

        IrReceiver.begin(IR_RECEIVE_PIN);
        IrSender.begin(IR_SEND_PIN);

        displayUtils->addToTerminalDisplay("IRin on pin " + String(IR_RECEIVE_PIN));
        displayUtils->addToTerminalDisplay("IRout on pin " + String(IR_SEND_PIN));
    }

    void IRRemoteState::onExit()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
    }

    ButtonType IRRemoteState::mapEventTypeToButtonType(EventType eventType)
    {
        switch (eventType)
        {
        case EVENT_ACTION_ONE:
            return BUTTON_ACTION_ONE;
        case EVENT_ACTION_TWO:
            return BUTTON_ACTION_TWO;
        case EVENT_UP:
            return BUTTON_UP;
        case EVENT_DOWN:
            return BUTTON_DOWN;
        case EVENT_LEFT:
            return BUTTON_LEFT;
        case EVENT_RIGHT:
            return BUTTON_RIGHT;
        case EVENT_SELECT:
            return BUTTON_SELECT;
        default:
            return BUTTON_COUNT;
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
                if (buttonIRData[button].isValid)
                {
                    displayUtils->addToTerminalDisplay("Sending stored IR data for button...");
                    IrReceiver.stop();
                    IrSender.sendRaw(buttonIRData[button].rawData, buttonIRData[button].rawDataLength, 38);
                    IrReceiver.start();
                    displayUtils->addToTerminalDisplay("IR signal sent.");
                }
                else
                {
                    recordingButton = button;
                    displayUtils->addToTerminalDisplay("Recording IR signal for button...");
                }
            }
        }

        if (IrReceiver.decode())
        {
            if (recordingButton != BUTTON_COUNT)
            {
                uint8_t len = IrReceiver.decodedIRData.rawDataPtr->rawlen;

                if (len - 1 > MAX_RAW_DATA_SIZE)
                {
                    len = MAX_RAW_DATA_SIZE + 1;
                }

                for (uint8_t i = 1; i < len; i++)
                {
                    buttonIRData[recordingButton].rawData[i - 1] = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
                }
                buttonIRData[recordingButton].rawDataLength = len - 1;
                buttonIRData[recordingButton].isValid = true;

                displayUtils->addToTerminalDisplay("Stored IR signal for button.");
                recordingButton = BUTTON_COUNT;
            }
            else
            {
                displayUtils->addToTerminalDisplay("Unsupported IR signal received.");
            }

            IrReceiver.resume();
        } else if (IrReceiver.isIdle()) {
            IrReceiver.resume();
        }
    }

} // namespace NuggetsInc
