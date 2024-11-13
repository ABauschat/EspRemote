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

// Constructor
CloneNFCState::CloneNFCState()
    : currentTab(TAB_DATA),
      tagDetected(false),
      currentScrollLine(0),
      maxVisibleLines(8),
      displayUtils(nullptr),
      availableSpace("Unknown") {
        
    // Initialize NFCLogic with IRQ and RESET pins
    nfcLogic = new NFCLogic(PN532_IRQ, PN532_RESET);
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
}

// Destructor
CloneNFCState::~CloneNFCState() {
    delete nfcLogic;
    delete displayUtils;
}

// onEnter method
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

// onExit method
void CloneNFCState::onExit() {
    // Any necessary cleanup can be done here
}

// update method
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
        else if (event.type == EVENT_SELECT) {
            if (tagDetected) {
                if (currentTab == TAB_INFO) {
                    // Initiate cloning process
                    displayUtils->displayMessage("Cloning in progress...");
                    delay(1000); // Allow message to display

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
                else {
                    // Toggle between tabs
                    currentTab = (currentTab == TAB_DATA) ? TAB_INFO : TAB_DATA;
                    displayUtils->displayTagInfo(clonedTagType, clonedData, parsedRecords, availableSpace, uidLength, uid, currentTab, dataLines, currentScrollLine, maxVisibleLines);
                }
            }
        }
    }

    if (!tagDetected) {
        readNFCTag();
    }
}

// Read NFC Tag
void CloneNFCState::readNFCTag() {
    Serial.println("Attempting to read NFC Tag...");
    if (nfcLogic->readTag(uid, &uidLength)) {
        tagDetected = true;

        clonedTagType = nfcLogic->getTagType(uidLength);
        clonedData = nfcLogic->readTagData(clonedTagType, uid, uidLength);

        if (clonedTagType == TAG_TYPE_NTAG2XX) {
            // Parse NDEF records
            // Convert clonedData string to raw bytes
            std::vector<uint8_t> rawData;
            for (size_t i = 0; i < clonedData.length(); i += 3) { // Assuming "XX " format
                if (i + 2 > clonedData.length()) break;
                String byteStr = clonedData.substring(i, i + 2);
                rawData.push_back(static_cast<uint8_t>(strtol(byteStr.c_str(), NULL, 16)));
            }
            parsedRecords = nfcLogic->parseNDEF(rawData.data(), rawData.size());
        }

        // Determine available space based on tag type
        if (clonedTagType == TAG_TYPE_NTAG2XX) {
            availableSpace = "888 bytes"; // Example for NTAG216
        }
        else {
            availableSpace = "Unknown";
        }

        splitDataIntoLines(clonedData);
        displayUtils->displayTagInfo(clonedTagType, clonedData, parsedRecords, availableSpace, uidLength, uid, currentTab, dataLines, currentScrollLine, maxVisibleLines);
    }
    else {
        displayUtils->displayMessage("No NFC Tag Detected");
    }
}

// Clone Tag Data
bool CloneNFCState::cloneTagData() {
    uint8_t targetUID[7];
    uint8_t targetUIDLength = 0;
    bool targetDetected = nfcLogic->readTag(targetUID, &targetUIDLength);

    if (!targetDetected) {
        displayUtils->displayMessage("No target tag detected");
        delay(2000);
        return false;
    }

    String targetTagType = nfcLogic->getTagType(targetUIDLength);

    if (clonedTagType != targetTagType) {
        displayUtils->displayMessage("Tag types do not match");
        delay(2000);
        return false;
    }

    if (clonedTagType == TAG_TYPE_MIFARE_CLASSIC) {
        return nfcLogic->cloneTagData(clonedTagType, targetUID, targetUIDLength, dataLines);
    }
    else if (clonedTagType == TAG_TYPE_NTAG2XX) {
        return nfcLogic->cloneTagData(clonedTagType, targetUID, targetUIDLength, dataLines);
    }
    else {
        displayUtils->displayMessage("Unsupported tag type for cloning");
        delay(2000);
        return false;
    }

    return false;
}

// Handle scrolling
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

// Split data into lines for display
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
