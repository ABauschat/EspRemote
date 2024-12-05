// IRRemoteState.cpp
#include "IRRemoteState.h"
#include "DisplayUtils.h"
#include <IRremote.hpp>
#include "Device.h"
#include "EventManager.h"
#include <LittleFS.h> 
#include <Arduino.h>

#define IR_RECEIVE_PIN 14
#define IR_SEND_PIN 15

namespace NuggetsInc
{
    IRRemoteState::IRRemoteState()
        : displayUtils(nullptr),
          recordingButton(BUTTON_COUNT),
          lastPressTime(0),
          pressCount(0)
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

        IrReceiver.stop();
    }

    void IRRemoteState::onEnter()
    {
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
        displayUtils->newTerminalDisplay("Adafruit IR Transceiver");

        IrReceiver.begin(IR_RECEIVE_PIN);
        IrSender.begin(IR_SEND_PIN);

        displayUtils->addToTerminalDisplay("IRin on pin " + String(IR_RECEIVE_PIN));
        displayUtils->addToTerminalDisplay("IRout on pin " + String(IR_SEND_PIN));

        loadIRData();
    }

    void IRRemoteState::onExit()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }

        IrReceiver.stop();
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
        case EVENT_BACK:
            return BUTTON_BACK;
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
                unsigned long currentTime = millis();

                if (button == BUTTON_ACTION_TWO)
                {
                    if (currentTime - lastPressTime < doublePressThreshold)
                    {
                        pressCount++;
                    }
                    else
                    {
                        pressCount = 1;
                    }

                    lastPressTime = currentTime;

                    if (pressCount == 2)
                    {
                        handleDoublePress(button);
                        pressCount = 0;
                    }
                }
                else
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
                    // Using the MICROS_PER_TICK defined in IRremote.hpp
                    buttonIRData[recordingButton].rawData[i - 1] = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
                }
                buttonIRData[recordingButton].rawDataLength = len - 1;
                buttonIRData[recordingButton].isValid = true;

                displayUtils->addToTerminalDisplay("Stored IR signal for button.");
                recordingButton = BUTTON_COUNT;
            }

            IrReceiver.resume();
        }
        else if (IrReceiver.isIdle())
        {
            IrReceiver.resume();
        }
    }

    void IRRemoteState::handleDoublePress(ButtonType button)
    {
        displayUtils->addToTerminalDisplay("Double press detected. Saving IR data to flash...");

        // Initialize LittleFS
        if (!LittleFS.begin(true))
        {
            displayUtils->addToTerminalDisplay("Failed to mount LittleFS.");
            return;
        }

        // Open the file for writing
        File file = LittleFS.open("/irData.bin", FILE_WRITE);
        if (!file)
        {
            displayUtils->addToTerminalDisplay("Failed to open file for writing.");
            return;
        }

        // Write Magic Number
        file.write(reinterpret_cast<const uint8_t*>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));

        // Write the buttonIRData array to the file
        size_t bytesWritten = file.write(reinterpret_cast<const uint8_t*>(buttonIRData), sizeof(buttonIRData));
        if (bytesWritten != sizeof(buttonIRData))
        {
            displayUtils->addToTerminalDisplay("Failed to write all IR data to flash.");
        }
        else
        {
            displayUtils->addToTerminalDisplay("IR data successfully saved to flash.");
        }

        file.close();
    }

    bool IRRemoteState::loadIRData()
    {
        displayUtils->addToTerminalDisplay("Loading IR data from flash...");

        // Initialize LittleFS
        if (!LittleFS.begin(false))
        {
            displayUtils->addToTerminalDisplay("Failed to mount LittleFS.");
            return false;
        }

        File file = LittleFS.open("/irData.bin", FILE_READ);
        if (!file)
        {
            displayUtils->addToTerminalDisplay("No IR data file found.");
            return false;
        }

        // Read Magic Number
        uint32_t fileMagic = 0;
        if (file.read(reinterpret_cast<uint8_t*>(&fileMagic), sizeof(fileMagic)) != sizeof(fileMagic))
        {
            displayUtils->addToTerminalDisplay("Failed to read Magic Number.");
            file.close();
            return false;
        }

        if (fileMagic != MAGIC_NUMBER)
        {
            displayUtils->addToTerminalDisplay("Invalid Magic Number. Data may be corrupted.");
            file.close();
            return false;
        }

        size_t bytesRead = file.read(reinterpret_cast<uint8_t*>(buttonIRData), sizeof(buttonIRData));
        if (bytesRead != sizeof(buttonIRData))
        {
            displayUtils->addToTerminalDisplay("Failed to read all IR data from flash.");
            file.close();
            return false;
        }

        // Mark all buttons as valid
        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            buttonIRData[i].isValid = true;
        }

        displayUtils->addToTerminalDisplay("IR data successfully loaded from flash.");
        file.close();
        return true;
    }

} // namespace NuggetsInc
