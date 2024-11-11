#include "CloneNFCState.h"
#include "Device.h"
#include "EventManager.h"
#include "Application.h"
#include "StateFactory.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

#define CLONE_SELECT_TIMEOUT 5000 // Timeout in ms to wait for target tag

namespace NuggetsInc {

CloneNFCState::CloneNFCState()
    : nfc(PN532_IRQ, PN532_RESET),
      tagDetected(false), 
      uidLength(0),
      totalDataLines(0), 
      currentScrollLine(0), 
      maxVisibleLines(8) // Adjust based on your display's size
{
    gfx = Device::getInstance().getDisplay();
}

CloneNFCState::~CloneNFCState() {}

void CloneNFCState::onEnter() {
    NewTerminalDisplay("Verifying NFC chip");
    Wire.begin(42, 45); // SDA, SCL
    AddToTerminalDisplay("I2C interface initialized");

    nfc.begin();
    AddToTerminalDisplay("NFC module initialized");

    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        displayMessage("PN532 not found");
        delay(2000);
        Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
        return;
    }

    Serial.print("Found chip PN5"); Serial.println((versiondata >> 24) & 0xFF, HEX);
    Serial.print("Firmware ver. "); Serial.print((versiondata >> 16) & 0xFF, DEC);
    Serial.print('.'); Serial.println((versiondata >> 8) & 0xFF, DEC);

    nfc.SAMConfig();
    AddToTerminalDisplay("SAMConfig completed");

    delay(500);
    displayMessage("Ready to read NFC tag");
    tagDetected = false;
}

void CloneNFCState::onExit() {
    nfc.SAMConfig();
    nfc.wakeup();
    nfc.reset();
    delay(100);
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
        else if (event.type == EVENT_SELECT) {
            if (tagDetected && !clonedData.isEmpty()) {
                if (cloneTagData()) {
                    displayMessage("Clone Successful!");
                }
                else {
                    displayMessage("Clone Failed!");
                }
                delay(2000);
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            }
        }
    }

    if (!tagDetected) {
        readNFCTag();
    }
}

void CloneNFCState::displayMessage(const String& message) {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 60);
    gfx->println(message);
}

void CloneNFCState::NewTerminalDisplay(const String& message) {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(0, 60);
    gfx->println(message);
    delay(100);
}

void CloneNFCState::AddToTerminalDisplay(const String& message) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->println(message);
    delay(100);
}

void CloneNFCState::displayTagInfo(const String& tagType, const String& tagData) {
    gfx->fillScreen(COLOR_BLACK);

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_ORANGE);
    gfx->setCursor(10, 10);
    gfx->println("Type:");

    gfx->setTextColor(COLOR_ORANGE);
    gfx->setCursor(80, 10);
    gfx->println(tagType);

    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_GREEN);
    gfx->setCursor(10, 40);
    gfx->println("UID:");

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_GREEN);
    gfx->setCursor(80, 40);
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) {
            uidStr += "0"; // Leading zero
        }
        uidStr += String(uid[i], HEX);
        uidStr += " ";
    }
    gfx->println(uidStr);

    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(10, 70);
    gfx->println("Data:");

    gfx->setTextSize(1);
    for (int i = 0; i < maxVisibleLines; i++) {
        int lineIndex = currentScrollLine + i;
        if (lineIndex < totalDataLines) {
            gfx->println(dataLines[lineIndex]);
        }
        else {
            gfx->println();
        }
    }

    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHEAT_CREAM);
    gfx->println("\nUse Up/Down to scroll");
    gfx->println("Press Select to Clone");
    gfx->println("Press Back to return");
}

void CloneNFCState::handleScroll(EventType eventType) {
    if (eventType == EVENT_UP) {
        if (currentScrollLine > 0) {
            currentScrollLine--;
            displayTagInfo(clonedTagType, clonedData);
        }
    }
    else if (eventType == EVENT_DOWN) {
        if (currentScrollLine + maxVisibleLines < totalDataLines) {
            currentScrollLine++;
            displayTagInfo(clonedTagType, clonedData);
        }
    }
}

bool CloneNFCState::cloneTagData() {
    displayMessage("Place target tag to clone");
    delay(2000);

    uint8_t targetUID[7];
    uint8_t targetUIDLength = 0;
    bool targetDetected = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, targetUID, &targetUIDLength);

    if (!targetDetected) {
        displayMessage("No target tag detected");
        delay(2000);
        return false;
    }

    String targetTagType = getTagType();

    if (clonedTagType != targetTagType) {
        displayMessage("Tag types do not match");
        delay(2000);
        return false;
    }

    if (clonedTagType == TAG_TYPE_MIFARE_CLASSIC) {
        return writeMIFAREClassic(targetUID, targetUIDLength);
    }
    else if (clonedTagType == TAG_TYPE_NTAG2XX) {
        return writeNTAG2xx(targetUID, targetUIDLength);
    }
    else {
        displayMessage("Unsupported tag type for cloning");
        delay(2000);
        return false;
    }

    return false;
}

void CloneNFCState::readNFCTag() {
    Serial.println("Attempting to read NFC Tag...");
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        tagDetected = true;

        String tagType = getTagType();
        String tagData = readTagData(tagType);

        clonedTagType = tagType;
        clonedData = tagData;

        splitDataIntoLines(tagData);
        displayTagInfo(tagType, tagData);
    }
    else {
        displayMessage("No NFC Tag Detected");
    }
}

String CloneNFCState::getTagType() {
    if (uidLength == 4) return TAG_TYPE_MIFARE_CLASSIC;
    if (uidLength == 7) return TAG_TYPE_NTAG2XX;
    return TAG_TYPE_UNKNOWN;
}

String CloneNFCState::readTagData(const String& tagType) {
    if (tagType == TAG_TYPE_MIFARE_CLASSIC) {
        return readMIFAREClassic();
    }
    else if (tagType == TAG_TYPE_NTAG2XX) {
        return readNTAG2xx();
    }
    return "Data reading not supported for this tag type.";
}

String CloneNFCState::readMIFAREClassic() {
    String dataStr = "";
    uint8_t sectorCount = 16;
    uint8_t blocksPerSector = 4;
    uint8_t defaultKeyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    for (uint8_t sector = 1; sector < sectorCount; sector++) {
        for (uint8_t block = 0; block < blocksPerSector - 1; block++) {
            uint8_t blockAddr = sector * blocksPerSector + block;
            uint8_t blockData[16];

            bool authenticated = nfc.mifareclassic_AuthenticateBlock(uid, uidLength, blockAddr, MIFARE_CMD_AUTH_A, defaultKeyA);
            if (!authenticated) {
                dataStr += "Auth Failed: Sector " + String(sector) + ", Block " + String(blockAddr) + "\n";
                continue;
            }

            bool success = nfc.mifareclassic_ReadDataBlock(blockAddr, blockData);
            if (success) {
                for (uint8_t i = 0; i < 16; i++) {
                    char hexByte[3];
                    sprintf(hexByte, "%02X", blockData[i]);
                    dataStr += String(hexByte) + " ";
                }
                dataStr += "\n";
            } else {
                dataStr += "Read Failed: Sector " + String(sector) + ", Block " + String(blockAddr) + "\n";
            }
        }
    }
    return dataStr;
}

String CloneNFCState::readNTAG2xx() {
    String dataStr = "";
    uint8_t totalPages = 39;
    uint8_t data[4];

    for (uint8_t page = 4; page < totalPages; page++) {
        bool success = nfc.ntag2xx_ReadPage(page, data);
        if (success) {
            for (uint8_t i = 0; i < 4; i++) {
                char hexByte[3];
                sprintf(hexByte, "%02X", data[i]);
                dataStr += String(hexByte) + " ";
            }
            dataStr += "\n";
        } else {
            dataStr += "Read Failed: Page " + String(page) + "\n";
        }
    }
    return dataStr;
}

bool CloneNFCState::writeMIFAREClassic(uint8_t* targetUID, uint8_t targetUIDLength) {
    Serial.println("Cloning MIFARE Classic tag...");
    uint8_t sectorCount = 16;
    uint8_t blocksPerSector = 4;
    uint8_t defaultKeyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    for (uint8_t sector = 1; sector < sectorCount; sector++) {
        for (uint8_t block = 0; block < blocksPerSector - 1; block++) {
            uint8_t blockAddr = sector * blocksPerSector + block;

            bool authenticated = nfc.mifareclassic_AuthenticateBlock(targetUID, targetUIDLength, blockAddr, MIFARE_CMD_AUTH_A, defaultKeyA);
            if (!authenticated) {
                Serial.print("Authentication failed for block ");
                Serial.println(blockAddr);
                displayMessage("Clone Failed: Auth Error");
                delay(2000);
                resetNFC();
                return false;
            }

            String dataLine = dataLines[blockAddr];
            uint8_t blockData[16];
            memset(blockData, 0, 16);
            for (uint8_t i = 0; i < 16 && (i * 3 + 2) < dataLine.length(); i++) {
                String byteStr = dataLine.substring(i * 3, i * 3 + 2);
                blockData[i] = strtol(byteStr.c_str(), NULL, 16);
            }

            if (!writeAndVerifyMIFAREClassicBlock(blockAddr, blockData)) {
                displayMessage("Clone Failed: Write Error");
                delay(2000);
                resetNFC();
                return false;
            }
        }
    }

    displayMessage("MIFARE Classic Clone Successful!");
    Serial.println("MIFARE Classic tag cloned successfully.");
    return true;
}

bool CloneNFCState::writeAndVerifyMIFAREClassicBlock(uint8_t blockAddr, uint8_t* blockData) {
    int retryCount = 3;
    while (retryCount-- > 0) {
        if (nfc.mifareclassic_WriteDataBlock(blockAddr, blockData)) {
            uint8_t verifyData[16];
            if (nfc.mifareclassic_ReadDataBlock(blockAddr, verifyData) && memcmp(verifyData, blockData, 16) == 0) {
                return true;
            }
        }
        delay(500);
    }
    return false;
}

bool CloneNFCState::writeNTAG2xx(uint8_t* targetUID, uint8_t targetUIDLength) {
    Serial.println("Cloning NTAG2xx tag...");

    if (dataLines[0].length() < 2) {
        displayMessage("Invalid NDEF URI Data");
        delay(2000);
        return false;
    }

    String uriLine = dataLines[0];
    String uri = "";
    for (uint8_t i = 0; i < uriLine.length(); i += 3) {
        if (i + 2 > uriLine.length()) break;
        String byteStr = uriLine.substring(i, i + 2);
        uri += (char)strtol(byteStr.c_str(), NULL, 16);
    }

    uint8_t uriPrefix = 0x01;
    String ndefRecord = "";
    ndefRecord += (char)0xD1;
    ndefRecord += (char)0x01;
    int payloadLength = 1 + uri.length();
    ndefRecord += (char)payloadLength;
    ndefRecord += (char)0x55;
    ndefRecord += (char)uriPrefix;
    ndefRecord += uri;

    int totalBytes = ndefRecord.length();
    int pagesNeeded = (totalBytes + 3) / 4;
    uint8_t buffer[4 * pagesNeeded];
    memset(buffer, 0, sizeof(buffer));

    for (int i = 0; i < ndefRecord.length(); i++) {
        buffer[i] = ndefRecord[i];
    }

    for (int page = 4; page < (4 + pagesNeeded); page++) {
        if (!nfc.ntag2xx_WritePage(page, &buffer[(page - 4) * 4])) {
            Serial.print("Failed to write page "); Serial.println(page);
            displayMessage("Clone Failed: Write Error");
            delay(2000);
            return false;
        }

        uint8_t verifyData[4];
        if (!nfc.ntag2xx_ReadPage(page, verifyData) || memcmp(verifyData, &buffer[(page - 4) * 4], 4) != 0) {
            Serial.print("Verification failed for page "); Serial.println(page);
            displayMessage("Clone Failed: Verification Error");
            delay(2000);
            return false;
        }
    }

    Serial.println("NDEF URI written and verified successfully.");
    displayMessage("NTAG2xx Clone Successful!");
    return true;
}

void CloneNFCState::resetNFC() {
    Serial.println("Resetting NFC module...");
    nfc.begin();
    nfc.SAMConfig();
    delay(500);
    displayMessage("NFC Module Reset");
}

void CloneNFCState::splitDataIntoLines(const String& tagData) {
    totalDataLines = 0;
    currentScrollLine = 0;
    for (int i = 0; i < MAX_DATA_LINES; i++) dataLines[i] = "";

    int start = 0;
    int end = tagData.indexOf('\n', start);
    while (end != -1 && totalDataLines < MAX_DATA_LINES) {
        dataLines[totalDataLines++] = tagData.substring(start, end);
        start = end + 1;
        end = tagData.indexOf('\n', start);
    }
    if (start < tagData.length() && totalDataLines < MAX_DATA_LINES) {
        dataLines[totalDataLines++] = tagData.substring(start);
    }
}

} // namespace NuggetsInc
