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
#include "TagDataHelper.h"

namespace NuggetsInc
{

    CloneNFCState::CloneNFCState()
        : currentTab(TAB_DATA),
          tagDetected(false),
          currentScrollLine(0),
          maxVisibleLines(5),
          displayUtils(nullptr),
          availableSpace("Unknown")
    {

        nfcLogic = new NFCLogic(PN532_IRQ, PN532_RESET);
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
    }

    CloneNFCState::~CloneNFCState()
    {
        delete nfcLogic;
        delete displayUtils;
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
            else if (event.type == EVENT_UP || event.type == EVENT_DOWN)
            {
                handleScroll(event.type);
            }
            else if (event.type == EVENT_LEFT || event.type == EVENT_RIGHT)
            {
                if (tagDetected)
                {
                    currentTab = (currentTab == TAB_DATA) ? TAB_INFO : TAB_DATA;
                    displayUtils->displayTagInfo(clonedTagType, clonedData, parsedRecords, availableSpace, uidLength, uid, currentTab, dataLines, currentScrollLine, maxVisibleLines);
                }
            }

            else if (event.type == EVENT_SELECT)
            {
                if (tagDetected)
                {
                    displayUtils->displayMessage("Cloning in progress...");
                    delay(1000);

                    if (cloneTagData())
                    {
                        displayUtils->displayMessage("Clone Successful!");
                    }
                    else
                    {
                        displayUtils->displayMessage("Clone Failed!");
                    }
                    delay(2000);
                    Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                    return;
                }
            }
        }

        if (!tagDetected)
        {
            readNFCTag();
        }
    }

    void CloneNFCState::readNFCTag()
    {
        if (nfcLogic->isTagPresent())
        {
            displayUtils->displayMessage("NFC Tag Detected: Keep steady");
            TagDataHelper tagHelper;
            const std::vector<uint8_t>& rawData = nfcLogic->readRawData();
            TagDataHelper NewtagData = tagHelper.parseRawData(rawData);

            int validationCode = tagHelper.ValidateTagData(NewtagData);

            if (validationCode != 0)
            {
                /*
                String rawDataStr;
                for (uint8_t byte : rawData) {
                    rawDataStr += String(byte, HEX) + " ";
                }
                displayUtils->displayMessage("Err Code: " + String(validationCode) + " Invalid Tag " + rawDataStr);
                return;
                */

                displayUtils->displayMessage("Un-Supported Tag");
            }
            

            displayUtils->displayMessage("Verified NFC Tag");
            delay(2000);
        }
        else
        {
            displayUtils->displayMessage("Searching for NFC Tag");
            delay(100);
        }
    }

    bool CloneNFCState::cloneTagData()
    {
        if (!nfcLogic->validateAndCloneTag(clonedTagType, dataLines))
        {
            displayUtils->displayMessage("Clone Failed!");
            delay(2000);
            return false;
        }

        return true;
    }

    void CloneNFCState::handleScroll(EventType eventType)
    {
        if (currentTab == TAB_DATA)
        {
            if (eventType == EVENT_UP)
            {
                if (currentScrollLine > 0)
                {
                    currentScrollLine--;
                    displayUtils->displayDataTab(dataLines, currentScrollLine, maxVisibleLines);
                }
            }
            else if (eventType == EVENT_DOWN)
            {
                if (currentScrollLine + maxVisibleLines < dataLines.size())
                {
                    currentScrollLine++;
                    displayUtils->displayDataTab(dataLines, currentScrollLine, maxVisibleLines);
                }
            }
        }
        else if (currentTab == TAB_INFO)
        {
        }
    }

    void CloneNFCState::splitDataIntoLines(const String &tagData)
    {
        dataLines.clear();
        currentScrollLine = 0;

        int start = 0;
        int end = tagData.indexOf('\n', start);
        while (end != -1)
        {
            dataLines.push_back(tagData.substring(start, end));
            start = end + 1;
            end = tagData.indexOf('\n', start);
        }
        if (start < tagData.length())
        {
            dataLines.push_back(tagData.substring(start));
        }
    }

} // namespace NuggetsInc
