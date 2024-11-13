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

namespace NuggetsInc {

CloneNFCState::CloneNFCState()
    : currentTab(TAB_DATA),
      tagDetected(false),
      currentScrollLine(0),
      maxVisibleLines(8),
      displayUtils(nullptr),
      availableSpace("Unknown") {
        
    nfcLogic = new NFCLogic(PN532_IRQ, PN532_RESET);
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
}

CloneNFCState::~CloneNFCState() {
    delete nfcLogic;
    delete displayUtils;
}

void CloneNFCState::onEnter() {
    displayUtils->newTerminalDisplay("Verifying NFC chip");

    if (!nfcLogic->initialize()) {
        displayUtils->displayMessage("PN532 not found");
        delay(2000);
        Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
        return;
    }

    displayUtils->addToTerminalDisplay("NFC module initialized");
    displayUtils->displayMessage("Ready to read NFC tag");
    tagDetected = false;
}

void CloneNFCState::onExit() {
   
}

void CloneNFCState::update() {
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    while (eventManager.getNextEvent(event)) {
        if (event.type == EVENT_BACK) {
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            return;
        }
        else if (event.type == EVENT_UP || event.type == EVENT_DOWN) {
            handleScroll(event.type);
        }
        else if (event.type == EVENT_LEFT || event.type == EVENT_RIGHT)
        {
            if (tagDetected) {
                currentTab = (currentTab == TAB_DATA) ? TAB_INFO : TAB_DATA;
                displayUtils->displayTagInfo(clonedTagType, clonedData, parsedRecords, availableSpace, uidLength, uid, currentTab, dataLines, currentScrollLine, maxVisibleLines);
            }
        } 

        else if (event.type == EVENT_SELECT) {
            if (tagDetected) {
                if (currentTab == TAB_INFO) {
                    displayUtils->displayMessage("Cloning in progress...");
                    delay(1000); 

                    if (cloneTagData()) {
                        displayUtils->displayMessage("Clone Successful!");
                    }
                    else {
                        displayUtils->displayMessage("Clone Failed!");
                    }
                    delay(2000);
                    Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                    return;
                }
            }
        }
    }

    if (!tagDetected) {
        readNFCTag();
    }
}

void CloneNFCState::readNFCTag() {
    TagData tagData;

    if (nfcLogic->readAndParseTagData(tagData)) {
        tagDetected = true;
        this->clonedTagType = tagData.tagType;
        this->clonedData = tagData.data;
        this->parsedRecords = tagData.ndefRecords;
        this->availableSpace = tagData.availableSpace;
        memcpy(this->uid, tagData.uid, tagData.uidLength);
        this->uidLength = tagData.uidLength;

        splitDataIntoLines(clonedData);
        displayUtils->displayTagInfo(clonedTagType, clonedData, parsedRecords, availableSpace, uidLength, uid, currentTab, dataLines, currentScrollLine, maxVisibleLines);
    }
    else {
        displayUtils->displayMessage("No NFC Tag Detected");
    }
}

bool CloneNFCState::cloneTagData() {
    if (!nfcLogic->validateAndCloneTag(clonedTagType, dataLines)) {
        displayUtils->displayMessage("Clone Failed!");
        delay(2000);
        return false;
    }

    return true;
}

void CloneNFCState::handleScroll(EventType eventType) {
    if (currentTab == TAB_DATA) {
        if (eventType == EVENT_UP) {
            if (currentScrollLine > 0) {
                currentScrollLine--;
                displayUtils->displayDataTab(dataLines, currentScrollLine, maxVisibleLines);
            }
        }
        else if (eventType == EVENT_DOWN) {
            if (currentScrollLine + maxVisibleLines < dataLines.size()) {
                currentScrollLine++;
                displayUtils->displayDataTab(dataLines, currentScrollLine, maxVisibleLines);
            }
        }
    }
    else if (currentTab == TAB_INFO) {
    }
}

void CloneNFCState::splitDataIntoLines(const String& tagData) {
    dataLines.clear();
    currentScrollLine = 0;

    int start = 0;
    int end = tagData.indexOf('\n', start);
    while (end != -1) {
        dataLines.push_back(tagData.substring(start, end));
        start = end + 1;
        end = tagData.indexOf('\n', start);
    }
    if (start < tagData.length()) {
        dataLines.push_back(tagData.substring(start));
    }
}

} // namespace NuggetsInc
