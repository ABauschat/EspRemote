#include "CloneNFCState.h"
#include "Device.h"
#include "EventManager.h"
#include "Application.h"
#include "StateFactory.h"
#include <Wire.h>
#include <Adafruit_PN532.h>

namespace NuggetsInc {

#define CLONE_SELECT_TIMEOUT 5000 // Timeout in ms to wait for target tag

// Constructor
CloneNFCState::CloneNFCState()
    : nfc(PN532_IRQ, PN532_RESET),
      tagDetected(false), 
      uidLength(0),
      totalDataLines(0), 
      currentScrollLine(0), 
      maxVisibleLines(8), // Adjust based on your display's size
      currentTab(TAB_DATA),
      ndefLength(0)
{
    gfx = Device::getInstance().getDisplay();
}

// Destructor
CloneNFCState::~CloneNFCState() {}

// onEnter method
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

// onExit method
void CloneNFCState::onExit() {
    nfc.SAMConfig();
    nfc.wakeup();
    nfc.reset();
    delay(100);
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
                    displayMessage("Cloning in progress...");
                    delay(1000); // Allow message to display

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
                else {
                    // Toggle between tabs
                    currentTab = (currentTab == TAB_DATA) ? TAB_INFO : TAB_DATA;
                    displayTagInfo(clonedTagType, clonedData);
                }
            }
        }
    }

    if (!tagDetected) {
        readNFCTag();
    }
}

// Display a single message
void CloneNFCState::displayMessage(const String& message) {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 60);
    gfx->println(message);
    // No need to call gfx->flush() or gfx->display()
}

// Display a new terminal message
void CloneNFCState::NewTerminalDisplay(const String& message) {
    gfx->fillScreen(COLOR_BLACK);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(0, 60);
    gfx->println(message);
    delay(100);
    // No need to call gfx->flush() or gfx->display()
}

// Add to terminal display
void CloneNFCState::AddToTerminalDisplay(const String& message) {
    gfx->setTextColor(COLOR_WHITE);
    gfx->setTextSize(1);
    gfx->println(message);
    delay(100);
    // No need to call gfx->flush() or gfx->display()
}

// Display tag information with tabs
void CloneNFCState::displayTagInfo(const String& tagType, const String& tagData) {
    gfx->fillScreen(COLOR_BLACK);
    
    // Display Header
    gfx->setTextSize(2);
    gfx->setTextColor(COLOR_ORANGE);
    gfx->setCursor(10, 10);
    gfx->println("NFC Tag Info");

    // Display Tabs
    gfx->setTextSize(1);
    // Indicate current tab with different color
    gfx->setTextColor(currentTab == TAB_DATA ? COLOR_WHITE : COLOR_WHEAT_CREAM);
    gfx->print("Data\t");
    gfx->setTextColor(currentTab == TAB_INFO ? COLOR_WHITE : COLOR_WHEAT_CREAM);
    gfx->println("Info");

    // Display Content based on currentTab
    if (currentTab == TAB_DATA) {
        displayDataTab();
    }
    else if (currentTab == TAB_INFO) {
        displayInfoTab();
    }

    // Instructions
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHEAT_CREAM);
    gfx->println("\nPress Select to clone");
    gfx->println("Use Up/Down to scroll");
    gfx->println("Press Back to return");
}

// Display Data tab (raw bytes)
void CloneNFCState::displayDataTab() {
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_WHITE);
    gfx->setCursor(10, 40); // Adjust cursor position as needed
    for (int i = 0; i < maxVisibleLines; i++) {
        int lineIndex = currentScrollLine + i;
        if (lineIndex < totalDataLines) {
            gfx->println(dataLines[lineIndex]);
        }
        else {
            gfx->println();
        }
    }
    // No need to call gfx->flush() or gfx->display()
}

// Display Info tab (parsed records and info)
void CloneNFCState::displayInfoTab() {
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_GREEN);
    gfx->setCursor(10, 40); // Adjust cursor position as needed
    
    gfx->println("Tag Type: " + clonedTagType);
    
    // Display UID
    String uidStr = "";
    for (uint8_t i = 0; i < uidLength; i++) {
        if (uid[i] < 0x10) {
            uidStr += "0"; // Leading zero
        }
        uidStr += String(uid[i], HEX);
        uidStr += " ";
    }
    gfx->println("UID: " + uidStr);

    // Display Parsed Records
    gfx->println("\nParsed Records:");
    for (size_t i = 0; i < parsedRecords.size(); i++) {
        gfx->println(String(i + 1) + ". Type: " + parsedRecords[i].type);
        gfx->println("   Payload: " + parsedRecords[i].payload);
    }

    // Display Available Space
    String availableSpace = "Unknown";
    if (clonedTagType == TAG_TYPE_NTAG2XX) {
        // Example for NTAG216
        availableSpace = "888 bytes";
        // Adjust based on specific NTAG2xx variant
    }
    gfx->println("\nAvailable Space: " + availableSpace);
    // No need to call gfx->flush() or gfx->display()
}

// Handle scrolling in the current tab
void CloneNFCState::handleScroll(EventType eventType) {
    if (currentTab == TAB_DATA) {
        if (eventType == EVENT_UP) {
            if (currentScrollLine > 0) {
                currentScrollLine--;
                displayDataTab();
            }
        }
        else if (eventType == EVENT_DOWN) {
            if (currentScrollLine + maxVisibleLines < totalDataLines) {
                currentScrollLine++;
                displayDataTab();
            }
        }
    }
    else if (currentTab == TAB_INFO) {
        // If Info tab requires scrolling, implement here
        // For simplicity, assuming it's not scrollable
    }
}

// Clone tag data
bool CloneNFCState::cloneTagData() {
    displayMessage("Cloning in progress...");
    delay(1000); // Allow message to display

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

// Read NFC tag
void CloneNFCState::readNFCTag() {
    Serial.println("Attempting to read NFC Tag...");
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        tagDetected = true;

        String tagType = getTagType();
        String tagData = readTagData(tagType);

        clonedTagType = tagType;
        clonedData = tagData;

        if (tagType == TAG_TYPE_NTAG2XX) {
            // Read NDEF message
            // Read raw NDEF data from NTAG2xx
            memset(ndefData, 0, sizeof(ndefData));
            ndefLength = 0;
            bool success = false;

            // NTAG2xx pages start from page 4
            // Each page has 4 bytes
            // Read pages until end of NDEF message (ME bit set in NDEF message header)
            // For simplicity, read all pages from 4 to 38 (for NTAG216)
            // You can optimize by stopping early if ME bit is set

            for (uint8_t page = 4; page < 39; page++) {
                uint8_t pageData[4];
                if (nfc.ntag2xx_ReadPage(page, pageData)) {
                    memcpy(&ndefData[ndefLength], pageData, 4);
                    ndefLength += 4;
                }
                else {
                    Serial.print("Failed to read page ");
                    Serial.println(page);
                    break;
                }
            }

            // Parse NDEF records
            parseNDEF(ndefData, ndefLength);
        }

        splitDataIntoLines(tagData);
        displayTagInfo(tagType, tagData);
    }
    else {
        displayMessage("No NFC Tag Detected");
    }
}

// Get Tag Type based on UID length
String CloneNFCState::getTagType() {
    if (uidLength == 4) return TAG_TYPE_MIFARE_CLASSIC;
    if (uidLength == 7) return TAG_TYPE_NTAG2XX;
    return TAG_TYPE_UNKNOWN;
}

// Read tag data based on tag type
String CloneNFCState::readTagData(const String& tagType) {
    if (tagType == TAG_TYPE_MIFARE_CLASSIC) {
        return readMIFAREClassic();
    }
    else if (tagType == TAG_TYPE_NTAG2XX) {
        return readNTAG2xx();
    }
    return "Data reading not supported for this tag type.";
}

// Read MIFARE Classic data
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

// Read NTAG2xx data as raw bytes
String CloneNFCState::readNTAG2xx() {
    String dataStr = "";
    // Read all pages from 4 to 38 (for NTAG216)
    for (uint8_t page = 4; page < 39; page++) {
        uint8_t pageData[4];
        if (nfc.ntag2xx_ReadPage(page, pageData)) {
            for (uint8_t i = 0; i < 4; i++) {
                char hexByte[3];
                sprintf(hexByte, "%02X", pageData[i]);
                dataStr += String(hexByte) + " ";
            }
            dataStr += "\n";
        }
        else {
            dataStr += "Read Failed: Page " + String(page) + "\n";
        }
    }
    return dataStr;
}

// Clone MIFARE Classic tag
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

// Write and verify a MIFARE Classic block
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

// Clone NTAG2xx tag
bool CloneNFCState::writeNTAG2xx(uint8_t* targetUID, uint8_t targetUIDLength) {
    Serial.println("Cloning NTAG2xx tag...");

    if (parsedRecords.empty()) {
        displayMessage("No NDEF Records to clone");
        delay(2000);
        return false;
    }

    // For this example, we'll write the raw NDEF data to the target tag
    // Ensure that ndefData contains the correct NDEF message to write
    // NTAG2xx pages start at page 4

    for (size_t page = 4; page < 39 && (page - 4)*4 < ndefLength; page++) {
        uint8_t pageData[4];
        memset(pageData, 0, 4);

        for (uint8_t i = 0; i < 4; i++) {
            size_t byteIndex = (page - 4) * 4 + i;
            if (byteIndex < ndefLength) {
                pageData[i] = ndefData[byteIndex];
            }
            else {
                pageData[i] = 0x00; // Padding
            }
        }

        if (!nfc.ntag2xx_WritePage(page, pageData)) {
            Serial.print("Failed to write page ");
            Serial.println(page);
            displayMessage("Clone Failed: Write Error");
            delay(2000);
            resetNFC();
            return false;
        }

        // Verify written data
        uint8_t verifyData[4];
        if (!nfc.ntag2xx_ReadPage(page, verifyData)) {
            Serial.print("Failed to read back page ");
            Serial.println(page);
            displayMessage("Clone Failed: Verification Error");
            delay(2000);
            resetNFC();
            return false;
        }

        if (memcmp(pageData, verifyData, 4) != 0) {
            Serial.print("Verification failed for page ");
            Serial.println(page);
            displayMessage("Clone Failed: Verification Error");
            delay(2000);
            resetNFC();
            return false;
        }
    }

    displayMessage("NTAG2xx Clone Successful!");
    Serial.println("NTAG2xx tag cloned successfully.");
    return true;
}

// Reset NFC module
void CloneNFCState::resetNFC() {
    Serial.println("Resetting NFC module...");
    nfc.begin();
    nfc.SAMConfig();
    delay(500);
    displayMessage("NFC Module Reset");
}

// Split data into lines for display
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

// Parse NDEF records from raw data
void CloneNFCState::parseNDEF(const uint8_t* data, size_t length) {
    parsedRecords.clear();
    size_t index = 0;

    while (index < length) {
        if (index + 2 > length) break; // Not enough data

        uint8_t header = data[index];
        bool mb = header & 0x80;
        bool me = header & 0x40;
        bool cf = header & 0x20;
        bool sr = header & 0x10;
        bool il = header & 0x08;
        uint8_t tnflen = data[index + 1];
        size_t payloadLength = sr ? data[index + 2] : ((data[index + 2] << 8) | data[index + 3]);
        size_t typeOffset = sr ? 3 : 4;

        if (!sr && !cf) typeOffset = 4;

        if (index + typeOffset + tnflen > length) break; // Prevent overflow

        String type = "";
        for (uint8_t i = 0; i < tnflen; i++) {
            type += (char)data[index + typeOffset + i];
        }

        size_t payloadOffset = typeOffset + tnflen + (il ? 1 : 0);
        if (payloadOffset > length) break;

        const uint8_t* payload = &data[index + payloadOffset];
        String parsedPayload = "";

        if (type == "U") {
            parsedPayload = parseURI(payload, payloadLength);
        }
        else if (type == "T") {
            parsedPayload = parseText(payload, payloadLength);
        }
        else {
            // Handle other types or skip
            parsedPayload = "Unsupported Record Type";
        }

        NDEFRecord record;
        record.type = type;
        record.payload = parsedPayload;
        parsedRecords.push_back(record);

        // Move to the next record
        if (sr) {
            index += 3 + tnflen + payloadLength + (il ? 1 : 0);
        }
        else {
            index += 4 + tnflen + payloadLength + (il ? 1 : 0);
        }

        if (me) break; // End of message
    }
}

// Parse URI payload
String CloneNFCState::parseURI(const uint8_t* payload, size_t length) {
    if (length < 1) return "Invalid URI";

    uint8_t uriIdentifier = payload[0];
    String uri = "";

    // Common URI prefixes based on RFC 3986
    const char* uriPrefixes[] = {
        "", "http://www.", "https://www.", "http://", "https://",
        "tel:", "mailto:", "ftp://anonymous:anonymous@", "ftp://ftp.",
        "ftps://", "sftp://", "smb://", "nfs://", "ftp://",
        "dav://", "news:", "telnet://", "imap:", "rtsp://",
        "urn:", "pop:", "sip:", "sips:", "tftp:", "btspp://",
        "btl2cap://", "btgoep://", "tcpobex://", "irdaobex://",
        "file://", "urn:epc:id:", "urn:epc:tag:", "urn:epc:pat:",
        "urn:epc:raw:", "urn:epc:", "urn:nfc:"
    };

    if (uriIdentifier < sizeof(uriPrefixes)/sizeof(uriPrefixes[0])) {
        uri += uriPrefixes[uriIdentifier];
    }

    for (size_t i = 1; i < length; i++) {
        uri += (char)payload[i];
    }

    return uri;
}

// Parse Text payload
String CloneNFCState::parseText(const uint8_t* payload, size_t length) {
    if (length < 1) return "Invalid Text";

    uint8_t status = payload[0];
    bool isUTF16 = status & 0x80;
    uint8_t langLength = status & 0x3F;

    String text = "";
    for (size_t i = 1 + langLength; i < length; i++) {
        text += (char)payload[i];
    }

    return text;
}

} // namespace NuggetsInc
