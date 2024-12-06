// IRRemoteState.cpp

#include "IRRemoteState.h"
#include "DisplayUtils.h"
#include "Device.h"
#include <Arduino.h>
#include "IRCommon.h"

namespace NuggetsInc
{
    IRRemoteState::IRRemoteState()
        : displayUtils(nullptr),
          selectedSlot(0),
          slotSelected(false)
    {
        String loadResult = LoadIRData(remotes);
        Serial.println(loadResult);
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
        Serial.println("Entering IRRemoteState.");
        
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
        displayUtils->clearDisplay();
        displayUtils->setTextSize(2);
        displayUtils->setTextColor(WHITE);
        InitializeSendTask(remotes);
        promptSlotSelection();
    }

    void IRRemoteState::onExit()
    {
        Serial.println("Exiting IRRemoteState.");
        
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
                if (!slotSelected)
                {
                    handleSlotSelection(button);
                }
                else
                {
                    if (button >= BUTTON_COUNT)
                    {
                        continue;
                    }
                    else
                    {
                        if (remotes[selectedSlot].buttonIRData[button].isValid)
                        {
                            if (NuggetsInc::EnqueueSendRequest(button, selectedSlot))
                            {
                                displayUtils->addToTerminalDisplay("IR send request enqueued.");
                            }
                            else
                            {
                                displayUtils->addToTerminalDisplay("Failed to enqueue IR send request.");
                            }
                        }
                        else
                        {
                            displayUtils->addToTerminalDisplay("No IR data stored for button.");
                        }
                    }
                }
            }
        }
    }

    void IRRemoteState::promptSlotSelection()
    {
        Arduino_GFX *gfx = Device::getInstance().getDisplay();
        gfx->fillScreen(BLACK);

        gfx->setTextSize(2);
        gfx->setTextColor(WHITE);
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

    void IRRemoteState::handleSlotSelection(ButtonType button)
    {
        Arduino_GFX *gfx = Device::getInstance().getDisplay();
        bool selectionChanged = false;

        switch (button)
        {
        case BUTTON_DOWN:
            if (selectedSlot < MAX_REMOTE_SLOTS - 1)
            {
                selectedSlot++;
                selectionChanged = true;
                Serial.println("Selected slot incremented.");
            }
            break;
        case BUTTON_UP:
            if (selectedSlot > 0)
            {
                selectedSlot--;
                selectionChanged = true;
                Serial.println("Selected slot decremented.");
            }
            break;
        case BUTTON_ACTION_ONE:
            slotSelected = true;
            displayUtils->clearDisplay();
            displayUtils->displayMessage("Slot " + String(selectedSlot) + " selected.");
            Serial.println("Slot " + String(selectedSlot) + " selected.");
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
