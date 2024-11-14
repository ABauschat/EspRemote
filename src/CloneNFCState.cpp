// CloneNFCState.cpp
#include "CloneNFCState.h"
#include "Device.h"
#include "EventManager.h"
#include "Application.h"
#include "StateFactory.h"
#include "Colors.h"
#include "DisplayUtils.h"
#include "Config.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "TagData.h"
#include "Tab.h"

namespace NuggetsInc
{

    CloneNFCState::CloneNFCState()
        : tagDetected(false),
          cloneTagData(false),
          displayNeedsRefresh(false),
          currentTabWindow("Tag Info", {10, 10, 240, 160}, Device::getInstance().getDisplay()), // Initialize Tab
          displayUtils(nullptr)
    {
        nfcLogic = new NFCLogic(PN532_IRQ, PN532_RESET);
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
    }

    CloneNFCState::~CloneNFCState()
    {
        delete nfcLogic;
        delete displayUtils;
        delete currentTagData;
    }

    void CloneNFCState::onEnter()
    {
        displayUtils->newTerminalDisplay("Verifying NFC chip");

        if (!nfcLogic->initialize())
        {
            displayUtils->displayMessage("PN532 not found");
            delay(2000);
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            return;
        }

        displayUtils->addToTerminalDisplay("NFC module Found");
        tagDetected = false;
    }

    void CloneNFCState::onExit()
    {
    }

    void CloneNFCState::update()
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
            else if (event.type == EVENT_UP || event.type == EVENT_DOWN || event.type == EVENT_LEFT || event.type == EVENT_RIGHT)
            {
                handleScroll(event.type);
            }
            else if (event.type == EVENT_SELECT)
            {
                if (tagDetected && !cloneTagData)
                {
                    cloneTagData = true;
                    displayNeedsRefresh = true;
                }
                else if (tagDetected && cloneTagData)
                {
                    displayUtils->displayMessage("Cloning Tag Data...");
                }
            }
        }

        if (!tagDetected)
        {
            readNFCTag();
        }
        else if (!cloneTagData && displayNeedsRefresh)
        {
            displayTagInformation();
        }
        else if (cloneTagData)
        {
            displayUtils->displayMessage("Cloning Tag Data...");
        }
    }

    void CloneNFCState::displayTagInformation()
    {
        if (currentTagData == nullptr)
        {
            return;
        }

        // Clear existing lines
        currentTabWindow.RemoveAllLines();

        // Set tab style (e.g., numbered list)
        currentTabWindow.setStyle(STYLE_NUMBERED);

        // Add UID to the tab
        currentTabWindow.addLine("UID: " + String(currentTagData->interpretations.UIDHex.c_str()));

        // Add Manufacturer to the tab
        currentTabWindow.addLine("Manufacturer: " + String(currentTagData->interpretations.manufacturerHex.c_str()));

        // Show tag type based on the tagType value
        std::string tagType = (currentTagData->tagType == 213) ? "NTAG213" : (currentTagData->tagType == 215) ? "NTAG215"
                                                                                                              : "NTAG216";
        currentTabWindow.addLine("Tag Type: " + String(tagType.c_str()));

        // Show memory details
        currentTabWindow.addLine("Total User Memory: " + String(std::to_string(currentTagData->userMemory.totalUserMemoryBytes).c_str()) + " bytes");

        // Add the confirmation message at the end
        currentTabWindow.addLine("Press SELECT to Confirm, BACK to Cancel");

        // Refresh the tab to display the content
        currentTabWindow.refreshTab();

        displayNeedsRefresh = false;
    }

    bool CloneNFCState::cloneTag()
    {
        // Implement cloning logic here
        return true;
    }

    void CloneNFCState::handleScroll(EventType eventType)
    {
        switch (eventType)
        {
        case EVENT_UP:
            currentTabWindow.scrollUp();
            displayNeedsRefresh = true;
            break;
        case EVENT_DOWN:
            currentTabWindow.scrollDown();
            displayNeedsRefresh = true;
            break;
        case EVENT_LEFT:
            break;
        case EVENT_RIGHT:
            break;
        default:
            break;
        }
    }

    void CloneNFCState::readNFCTag()
    {
        if (nfcLogic->isTagPresent())
        {
            displayUtils->displayMessage("NFC Tag Detected: Keep steady");

            TagData tag;
            const std::vector<uint8_t> &rawData = nfcLogic->readRawData();
            TagData NewtagData = tag.parseRawData(rawData);

            int validationCode = tag.ValidateTagData(NewtagData);

            if (validationCode != 0)
            {
                // Play A Long Vibration to indicate an error
                displayUtils->displayMessage("Un-Supported Tag");
                delay(1500);
            }
            else
            {
                currentTagData = new TagData(NewtagData);
            }

            displayUtils->displayMessage("Verified NFC Tag");
            delay(1500);

            tagDetected = true;
            displayNeedsRefresh = true;
        }
        else
        {
            displayUtils->displayMessage("Searching for NFC Tag");
            delay(100);
        }
    }

} // namespace NuggetsInc
