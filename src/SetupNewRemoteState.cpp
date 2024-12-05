// SetupNewRemoteState.cpp

#include "SetupNewRemoteState.h"
#include "Device.h"
#include <LittleFS.h>
#include "IRCommon.h"

namespace NuggetsInc
{
    SetupNewRemoteState::SetupNewRemoteState()
        : displayUtils(nullptr),
          selectedSlot(0),
          slotSelected(false),
          recordingButton(BUTTON_COUNT),
          lastPressTime(0),
          pressCount(0)
    {
        LoadIRData(remotes);
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
        displayUtils->clearDisplay();
        displayUtils->setTextSize(2);
        displayUtils->setTextColor(COLOR_WHITE);

        promptSlotSelection();
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
                if (!slotSelected)
                {
                    handleSlotSelection(button);
                }
                else
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
                        
                            recordingButton = button;
                            displayUtils->addToTerminalDisplay("Recording IR signal for button...");
                            BeginIrReceiver();
                        
                    }
                }
            }
        }

        if (RecieverDecode())
        {
            if (recordingButton != BUTTON_COUNT)
            {
                remotes[selectedSlot].buttonIRData[recordingButton] = DecodeIRData();

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
        if (button == BUTTON_ACTION_TWO)
        {
            displayUtils->addToTerminalDisplay("Double press detected. Saving IR data to flash...");

            if (!LittleFS.begin(true))
            {
                displayUtils->addToTerminalDisplay("Failed to mount LittleFS.");
                return;
            }

            String filename = "/irData" + String(selectedSlot) + ".bin";
            File file = LittleFS.open(filename, FILE_WRITE);
            if (!file)
            {
                displayUtils->addToTerminalDisplay("Failed to open file for writing.");
                return;
            }

            file.write(reinterpret_cast<const uint8_t *>(&MAGIC_NUMBER), sizeof(MAGIC_NUMBER));

            size_t bytesWritten = file.write(reinterpret_cast<const uint8_t *>(remotes[selectedSlot].buttonIRData), sizeof(remotes[selectedSlot].buttonIRData));
            if (bytesWritten != sizeof(remotes[selectedSlot].buttonIRData))
            {
                displayUtils->addToTerminalDisplay("Failed to write all IR data to flash.");
            }
            else
            {
                displayUtils->addToTerminalDisplay("IR data successfully saved to flash.");
            }

            file.close();
        }
    }

    void SetupNewRemoteState::promptSlotSelection()
    {
        Arduino_GFX *gfx = Device::getInstance().getDisplay();
        gfx->fillScreen(COLOR_BLACK);

        gfx->setTextSize(2);
        gfx->setTextColor(COLOR_WHITE);
        gfx->setCursor(10, 10);
        gfx->println("Select Remote Slot:");

        for (uint8_t slot = 0; slot < MAX_REMOTE_SLOTS; slot++)
        {
            int yPosition = 40 + (slot * 20);
            if (slot == selectedSlot)
            {
                gfx->fillRect(5, yPosition - 5, 300, 20, COLOR_ORANGE);
            }

            String status = "Slot " + String(slot) + ": ";
            status += remotes[slot].buttonIRData[BUTTON_ACTION_ONE].isValid ? "Used" : "Empty";

            gfx->setCursor(10, yPosition);
            gfx->println(status);
        }
    }

    void SetupNewRemoteState::handleSlotSelection(ButtonType button)
    {
        bool selectionChanged = false;

        switch (button)
        {
        case BUTTON_DOWN:
            if (selectedSlot < MAX_REMOTE_SLOTS - 1)
            {
                selectedSlot++;
                selectionChanged = true;
            }
            break;
        case BUTTON_UP:
            if (selectedSlot > 0)
            {
                selectedSlot--;
                selectionChanged = true;
            }
            break;
        case BUTTON_ACTION_ONE:
            slotSelected = true;

            displayUtils->clearDisplay();
            displayUtils->displayMessage("Slot " + String(selectedSlot) + " selected.");
            break;
        default:
            break;
        }

        if (selectionChanged)
        {
            promptSlotSelection();
        }
    }

} // namespace NuggetsInc
