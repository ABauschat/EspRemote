// IRRemoteState.cpp
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
        // Initialize all IRData entries
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
        // Initialize DisplayUtils
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());

        displayUtils->newTerminalDisplay("Adafruit IR Transceiver");

        // Initialize IR Receiver and Sender
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

        // Check for events
        while (eventManager.getNextEvent(event))
        {
            ButtonType button = mapEventTypeToButtonType(event.type);
            if (button != BUTTON_COUNT)
            {
                if (buttonIRData[button].isValid)
                {
                    // Send the stored data
                    displayUtils->addToTerminalDisplay("Sending stored IR data for button...");
                    // Disable receiver before sending
                    IrReceiver.stop();

                    // Send the raw data
                    IrSender.sendRaw(buttonIRData[button].rawData, buttonIRData[button].rawDataLength, 38);

                    // Re-enable receiver after sending
                    IrReceiver.start();

                    displayUtils->addToTerminalDisplay("IR signal sent.");
                }
                else
                {
                    // Start recording for this button
                    recordingButton = button;
                    displayUtils->addToTerminalDisplay("Recording IR signal for button...");
                }
            }
        }

        // Decode IR signal
        if (IrReceiver.decode())
        {
            if (recordingButton != BUTTON_COUNT)
            {
                // Store the IR data to the recordingButton
                uint8_t len = IrReceiver.decodedIRData.rawDataPtr->rawlen;

                // Ensure length does not exceed the maximum size
                if (len - 1 > MAX_RAW_DATA_SIZE)
                {
                    len = MAX_RAW_DATA_SIZE + 1;
                }

                // Convert raw data from ticks to microseconds and store
                for (uint8_t i = 1; i < len; i++) // Start from 1 to skip initial gap
                {
                    buttonIRData[recordingButton].rawData[i - 1] = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
                }
                buttonIRData[recordingButton].rawDataLength = len - 1;
                buttonIRData[recordingButton].isValid = true;

                // Display message
                displayUtils->addToTerminalDisplay("Stored IR signal for button.");

                // Reset recordingButton
                recordingButton = BUTTON_COUNT;
            }
            else
            {
                // Not recording, ignore or display received IR signal
                displayUtils->addToTerminalDisplay("Received IR signal but not recording.");
            }

            // Prepare for the next IR signal
            IrReceiver.resume();
        }
    }

} // namespace NuggetsInc
