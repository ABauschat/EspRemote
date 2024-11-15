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
          currentTabIndex(0),
          displayUtils(nullptr),
          currentTabWindow("Default", DisplayArea{0, 0, 0, 0}, Device::getInstance().getDisplay())
    {
        nfcLogic = new NFCLogic(PN532_IRQ, PN532_RESET);
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());

        // Initialize tabs with proper display areas
        Arduino_GFX *display = Device::getInstance().getDisplay();

        // Define the virtual display area
        DisplayArea tabArea = {0, 0, 536, 150}; // Your specified area

        tabs.emplace_back("Info", tabArea, display);
        tabs.emplace_back("Raw Data", tabArea, display);
        tabs.emplace_back("ASCII", tabArea, display);
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
                cloneTagData = true;
            }
        }

        if (!tagDetected)
        {
            readNFCTag();
        }
        else if (!cloneTagData && displayNeedsRefresh)
        {
            displayTagInformation();
        } else if (cloneTagData)
        {
           cloneTag();
        }
    }

    void CloneNFCState::populateTabs()
    {
        if (currentTagData == nullptr)
        {
            return;
        }

        // Clear all tabs' lines
        for (auto &tab : tabs)
        {
            tab.RemoveAllLines();
        }

        // Populate the "Info" tab
        tabs[0].setStyle(STYLE_NUMBERED);
        tabs[0].addLine("UID: " + String(currentTagData->interpretations.UIDHex.c_str()));
        tabs[0].addLine("Manufacturer: " + String(currentTagData->interpretations.manufacturerHex.c_str()));
        std::string tagType = (currentTagData->tagType == 213) ? "NTAG213" : (currentTagData->tagType == 215) ? "NTAG215"
                                                                                                              : "NTAG216";
        tabs[0].addLine("Tag Type: " + String(tagType.c_str()));
        tabs[0].addLine("Total User Memory: " + String(std::to_string(currentTagData->userMemory.totalUserMemoryBytes).c_str()) + " bytes");

        // **Add the NDEF Records to the Info tab**
        tabs[0].addLine("NDEF Records:");
        for (size_t i = 0; i < currentTagData->records.size(); ++i)
        {
            const TagData::Record &record = currentTagData->records[i];
            tabs[0].addLine("Record " + String(i + 1) + ":");
            tabs[0].addLine("  Type: " + String(record.type.c_str()));

            // Convert payload to a printable string
            String payloadStr = "";
            for (uint8_t byte : record.payload)
            {
                if (byte >= 32 && byte <= 126)
                    payloadStr += char(byte);
                else
                    payloadStr += '.'; // Non-printable character placeholder
            }
            tabs[0].addLine("  Payload: " + payloadStr);
        }

        // Populate the "Raw Data" tab with hex data formatted 4 bytes at a time
        tabs[1].setStyle(STYLE_NUMBERED); // Set line style to numbered
        const std::vector<uint8_t> &rawData = currentTagData->rawData;
        for (size_t i = 0; i < rawData.size(); i += 4)
        {
            String line = "";
            for (size_t j = i; j < i + 4 && j < rawData.size(); ++j)
            {
                if (rawData[j] < 16)
                    line += "0"; // Add leading zero for single-digit hex values
                line += String(rawData[j], HEX) + " ";
            }
            tabs[1].addLine(line);
        }

        // Populate the "ASCII" tab with ASCII data formatted 4 bytes at a time
        tabs[2].setStyle(STYLE_NUMBERED); // Set line style to numbered
        for (size_t i = 0; i < rawData.size(); i += 4)
        {
            String asciiLine = "";
            for (size_t j = i; j < i + 4 && j < rawData.size(); ++j)
            {
                // ASCII representation
                if (rawData[j] >= 32 && rawData[j] <= 126)
                    asciiLine += char(rawData[j]);
                else
                    asciiLine += '.'; // Non-printable character placeholder
            }
            tabs[2].addLine(asciiLine);
        }
    }

    void CloneNFCState::displayTagInformation()
    {
        // Get the display instance
        Arduino_GFX *display = Device::getInstance().getDisplay();

        // Refresh the current tab to display the content
        tabs[currentTabIndex].refreshTab();

        // Draw the tab headers
        tabs[currentTabIndex].DrawTabHeaders(tabs, currentTabIndex);

        displayNeedsRefresh = false;
    }

    bool CloneNFCState::cloneTag()
    {

        // Check if a tag is still present before writing
        if (!nfcLogic->isTagPresent())
        {
            displayUtils->displayMessage("Searching For NFC Tag");
            return false;
        } else {
            displayUtils->displayMessage("NFC Tag Detected: Keep Steady");
        }

        if (currentTagData == nullptr)
        {
            displayUtils->displayMessage("No Tag Data Available");
            return false;
        }

        // Proceed to write tag data
        if (nfcLogic->writeTagData(*currentTagData))
        {
            displayUtils->displayMessage("Tag Cloned Successfully");
            delay(2000);
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
        }

        return false;
    }

    void CloneNFCState::handleScroll(EventType eventType)
    {
        switch (eventType)
        {
        case EVENT_UP:
            tabs[currentTabIndex].scrollUp();
            displayNeedsRefresh = true;
            break;
        case EVENT_DOWN:
            tabs[currentTabIndex].scrollDown();
            displayNeedsRefresh = true;
            break;
        case EVENT_LEFT:
            if (currentTabIndex > 0)
            {
                currentTabIndex--;
                tabs[currentTabIndex].setNeedsRefresh(true); // Add this line
                displayNeedsRefresh = true;
            }
            break;
        case EVENT_RIGHT:
            if (currentTabIndex < tabs.size() - 1)
            {
                currentTabIndex++;
                tabs[currentTabIndex].setNeedsRefresh(true); // Add this line
                displayNeedsRefresh = true;
            }
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
                // Play a long vibration to indicate an error
                displayUtils->displayMessage("Un-Supported Tag");
                delay(1500);
                return;
            }
            else
            {
                currentTagData = new TagData(NewtagData);
                // Populate the tabs once after tag is read
                populateTabs();
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
