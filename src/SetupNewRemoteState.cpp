// SetupNewRemoteState.cpp
#include "SetupNewRemoteState.h"
#include "DisplayUtils.h"
#include "Device.h"
#include <LittleFS.h>
#include <Arduino.h>

namespace NuggetsInc
{
    SetupNewRemoteState::SetupNewRemoteState()
        : displayUtils(nullptr),
          recordingButton(BUTTON_COUNT),
          lastPressTime(0),
          pressCount(0)
    {
    }

    SetupNewRemoteState::~SetupNewRemoteState()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
    }

    void SetupNewRemoteState::onEnter()
    {
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
        displayUtils->newTerminalDisplay("Adafruit IR Transceiver");

        BeginIrReceiver();
    }

    void SetupNewRemoteState::onExit()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }

        StopIrReceiver();
    }

    void SetupNewRemoteState::update()
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
                    }
                    else
                    {
                        recordingButton = button;
                        displayUtils->addToTerminalDisplay("Recording IR signal for button...");
                    }
                }
            }
        }

        if (RecieverDecode())
        {
            if (recordingButton != BUTTON_COUNT)
            {
                buttonIRData[recordingButton] = DecodeIRData();

                displayUtils->addToTerminalDisplay("Stored IR signal for button.");
                recordingButton = BUTTON_COUNT;
            }

            RecieverResume();
        }
        else if (!RecieverIsIdle())
        {
            RecieverResume();
        }
    }

    void SetupNewRemoteState::handleDoublePress(ButtonType button)
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
        file.write(reinterpret_cast<const uint8_t *>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));

        // Write the buttonIRData array to the file
        size_t bytesWritten = file.write(reinterpret_cast<const uint8_t *>(buttonIRData), sizeof(buttonIRData));
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

} // namespace NuggetsInc 