#include "CloneNFCState.h"
#include "Device.h"
#include "EventManager.h"
#include "Application.h"
#include "StateFactory.h"

#include <Wire.h>
#include <Adafruit_PN532.h>

// Define constants for tag types
#define TAG_TYPE_MIFARE_ULTRALIGHT "MIFARE Ultralight"
#define TAG_TYPE_MIFARE_CLASSIC "MIFARE Classic"
#define TAG_TYPE_FELICA "FeliCa"

// Define colors (Replace with actual color values based on your display's color format)
#define COLOR_ORANGE       0xFD20 // Example RGB565 value for orange
#define COLOR_GREEN        0x07E0 // RGB565 green
#define COLOR_WHEAT_CREAM  0xF7B9 // RGB565 light yellow (wheat cream)
#define COLOR_WHITE        0xFFFF // RGB565 white

namespace NuggetsInc {

CloneNFCState::CloneNFCState()
    : nfc(PN532_IRQ, PN532_RESET, &Wire), tagDetected(false), uidLength(0),
      totalDataLines(0), currentScrollLine(0), maxVisibleLines(8) {} // Adjust maxVisibleLines based on display size

CloneNFCState::~CloneNFCState() {}

void CloneNFCState::onEnter() {
    displayMessage("Searching for NFC chip...");

    // Initialize the I2C interface with correct pins
    Wire.begin(42, 45); // SDA, SCL

    nfc.begin();

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        displayMessage("PN532 not found");
        delay(2000);
        Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
        return;
    }

    nfc.SAMConfig(); // Configure the PN532 to read RFID tags
    tagDetected = false;
}

void CloneNFCState::onExit() {
    // Turn off the NFC module
    nfc.SAMConfig();
    nfc.wakeup();
    nfc.reset();
    delay(100);
}

void CloneNFCState::update() {
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    // Check for events
    while (eventManager.getNextEvent(event)) {
        if (event.type == EVENT_BACK) {
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            return;
        }
        else if (event.type == EVENT_UP || event.type == EVENT_DOWN) {
            handleScroll(event.type);
        }
    }

    if (!tagDetected) {
        readNFCTag();
    }
}

void CloneNFCState::displayMessage(const String& message) {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 60);
    gfx->println(message);
}

void CloneNFCState::readNFCTag() {
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        tagDetected = true;

        // Determine the tag type
        String tagType = getTagType();

        // Read data based on tag type
        String tagData = readTagData(tagType);

        // Split data into lines for scrolling
        splitDataIntoLines(tagData);

        // Display tag information and data
        displayTagInfo(tagType, "");
    }
    else {
        displayMessage("No NFC Tag Detected");
    }
}

String CloneNFCState::getTagType() {
    // Implement specific identification logic
    // Example placeholder
    if (uidLength == 4) return TAG_TYPE_MIFARE_ULTRALIGHT;
    if (uidLength == 7) return TAG_TYPE_MIFARE_CLASSIC;
    if (uidLength == 10) return TAG_TYPE_FELICA;
    return "Unknown";
}

String CloneNFCState::readTagData(const String& tagType) {
    if (tagType == TAG_TYPE_MIFARE_ULTRALIGHT) {
        return readMIFAREUltralight();
    }
    else if (tagType == TAG_TYPE_MIFARE_CLASSIC) {
        return readMIFAREClassic();
    }
    // Add more tag types as needed
    else {
        return "Data reading not supported for this tag type.";
    }
}

String CloneNFCState::readMIFAREUltralight() {
    // MIFARE Ultralight typically has 16 pages (4 bytes each)
    uint8_t pageData[4];
    String dataStr = "";

    // Attempt to read all 16 pages
    for (uint8_t page = 0; page < 16; page++) {
        if (nfc.ntag2xx_ReadPage(page, pageData)) {
            for (uint8_t i = 0; i < 4; i++) {
                // Convert byte to hex string
                char hexByte[3];
                sprintf(hexByte, "%02X", pageData[i]);
                dataStr += String(hexByte);
                dataStr += " ";
            }
            dataStr += "\n";
        }
        else {
            dataStr += "Read failed at page " + String(page) + "\n";
        }
    }

    return dataStr;
}

String CloneNFCState::readMIFAREClassic() {
    // MIFARE Classic tags have sectors and blocks
    // For simplicity, we'll read the first block of each sector

    String dataStr = "";
    uint8_t sectorCount = 16; // Depends on the tag version (e.g., 1K has 16 sectors)
    uint8_t blocksPerSector = 4; // Typically 4 blocks per sector

    for (uint8_t sector = 0; sector < sectorCount; sector++) {
        // Calculate block address
        uint8_t blockAddr = sector * blocksPerSector;

        uint8_t blockData[16];
        // Default key for MIFARE Classic is often 0xFFFFFFFFFFFF
        uint8_t defaultKey[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };

        // Authenticate the block
        if (nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockAddr, 0, defaultKey)) {
            // Read the block
            if (nfc.mifareclassic_ReadDataBlock(blockAddr, blockData)) {
                // Convert block data to hex string
                for (uint8_t i = 0; i < 16; i++) {
                    char hexByte[3];
                    sprintf(hexByte, "%02X", blockData[i]);
                    dataStr += String(hexByte);
                    dataStr += " ";
                }
                dataStr += "\n";
            }
            else {
                dataStr += "Read failed at block " + String(blockAddr) + "\n";
            }
        }
        else {
            dataStr += "Authentication failed for block " + String(blockAddr) + "\n";
        }
    }

    return dataStr;
}

void CloneNFCState::displayTagInfo(const String& tagType, const String& unused) {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);

    // Display "NFC Tag Found!" in white with text size 2
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 10);
    gfx->println("NFC Tag Found!");

    // Display "Type:" label in orange with text size 2
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_ORANGE);
    gfx->setCursor(10, 40);
    gfx->println("Type:");

    // Display actual tag type in orange with text size 2
    gfx->setTextColor(COLOR_ORANGE);
    gfx->setCursor(80, 40); // Adjust X position as needed
    gfx->println(tagType);

    // Display "UID:" label in green with text size 1
    gfx->setTextSize(1); // Approximate 1.5 by using size 1 for label and size 2 for value
    gfx->setTextColor(COLOR_GREEN);
    gfx->setCursor(10, 70);
    gfx->println("UID:");

    // Display actual UID in green with text size 2
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_GREEN);
    gfx->setCursor(80, 70); // Adjust X position as needed
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) {
            uidStr += "0"; // Leading zero
        }
        uidStr += String(uid[i], HEX);
        uidStr += " ";
    }
    gfx->println(uidStr);

    // Display "Data:" label in white with text size 1
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(10, 110);
    gfx->println("Data:");

    // Display visible data lines based on currentScrollLine
    gfx->setTextSize(1.5);
    for (int i = 0; i < maxVisibleLines; i++) {
        int lineIndex = currentScrollLine + i;
        if (lineIndex < totalDataLines) {
            gfx->println(dataLines[lineIndex]);
        }
        else {
            gfx->println(); // Empty line
        }
    }

    // Display scrolling instructions in wheat cream with text size 2
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHEAT_CREAM);
    gfx->println("\nUse Up/Down to scroll");
    gfx->println("Press Back to return");
}

void CloneNFCState::splitDataIntoLines(const String& tagData) {
    totalDataLines = 0;
    currentScrollLine = 0; // Reset scroll position

    // Clear existing dataLines
    for (int i = 0; i < MAX_DATA_LINES; i++) {
        dataLines[i] = "";
    }

    // Split tagData by newline characters
    int start = 0;
    int end = tagData.indexOf('\n', start);
    while (end != -1 && totalDataLines < MAX_DATA_LINES) {
        dataLines[totalDataLines++] = tagData.substring(start, end);
        start = end + 1;
        end = tagData.indexOf('\n', start);
    }
    // Add the last line if any
    if (start < tagData.length() && totalDataLines < MAX_DATA_LINES) {
        dataLines[totalDataLines++] = tagData.substring(start);
    }
}

void CloneNFCState::handleScroll(EventType eventType) {
    if (eventType == EVENT_UP) {
        if (currentScrollLine > 0) {
            currentScrollLine--;
            displayTagInfo(getTagType(), "");
        }
    }
    else if (eventType == EVENT_DOWN) {
        if (currentScrollLine + maxVisibleLines < totalDataLines) {
            currentScrollLine++;
            displayTagInfo(getTagType(), "");
        }
    }
}

} // namespace NuggetsInc
