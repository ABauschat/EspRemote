#ifndef CLONE_NFC_STATE_H
#define CLONE_NFC_STATE_H

#include "State.h"
#include <Adafruit_PN532.h>
#include <Arduino_GFX.h>
#include <vector>

namespace NuggetsInc {

// Define constants for tag types
#define TAG_TYPE_MIFARE_ULTRALIGHT "MIFARE Ultralight"
#define TAG_TYPE_MIFARE_CLASSIC "MIFARE Classic"
#define TAG_TYPE_NTAG2XX "NTAG2xx"
#define TAG_TYPE_UNKNOWN "Unknown"

// Define colors (Replace with actual color values based on your display's color format)
#define COLOR_ORANGE       0xFD20 // Example RGB565 value for orange
#define COLOR_GREEN        0x07E0 // RGB565 green
#define COLOR_WHEAT_CREAM  0xF7B9 // RGB565 light yellow (wheat cream)
#define COLOR_WHITE        0xFFFF // RGB565 white
#define COLOR_BLACK        0x0000 // RGB565 black

#define PN532_IRQ 39   
#define PN532_RESET 46

class CloneNFCState : public AppState {
public:
    CloneNFCState();
    ~CloneNFCState();

    void onEnter() override;
    void onExit() override;
    void update() override;


private:
    // Display functions
    void displayMessage(const String& message);
    void NewTerminalDisplay(const String& message);
    void AddToTerminalDisplay(const String& message);
    void displayTagInfo(const String& tagType, const String& tagData);

    void displayDataTab();
    void displayInfoTab();

    // NFC functions
    void readNFCTag();
    String getTagType();
    String readTagData(const String& tagType);
    String readMIFAREClassic();
    String readNTAG2xx();

    // Cloning functions
    bool cloneTagData();
    bool writeMIFAREClassic(uint8_t* targetUID, uint8_t targetUIDLength);
    bool writeNTAG2xx(uint8_t* targetUID, uint8_t targetUIDLength);
    bool writeNDEFText(const char* text);

    // Helper function to write NDEF URI
    bool writeNDEFURI(uint8_t uriIdentifier, const char* uri);

    // Utility functions
    void handleScroll(EventType eventType);
    void splitDataIntoLines(const String& tagData);
    void resetNFC();
    bool writeAndVerifyMIFAREClassicBlock(uint8_t blockAddr, uint8_t* blockData);

    // NDEF Parsing
    struct NDEFRecord {
        String type;
        String payload;
    };
    std::vector<NDEFRecord> parsedRecords;

    void parseNDEF(const uint8_t* data, size_t length);
    String parseURI(const uint8_t* payload, size_t length);
    String parseText(const uint8_t* payload, size_t length);

    // Display tabs
    enum DisplayTab {
        TAB_DATA,
        TAB_INFO
    };
    DisplayTab currentTab;

    // NFC variables
    Adafruit_PN532 nfc; // PN532 instance
    bool tagDetected;
    uint8_t uid[7];      // Buffer to store the returned UID
    uint8_t uidLength;   // Length of the UID (4 or 7 bytes)
    String tagData;      // Data read from the tag

    // Cloning variables
    String clonedData;
    String clonedTagType;

    // Raw NDEF data
    uint8_t ndefData[256];
    size_t ndefLength;

    // Scrolling variables
    static const int MAX_DATA_LINES = 50;  // Maximum number of data lines
    String dataLines[MAX_DATA_LINES];       // Array to store data lines
    int totalDataLines;                     // Total number of data lines
    int currentScrollLine;                  // Current top line displayed
    int maxVisibleLines;                    // Number of lines visible on screen

    // Display object
    Arduino_GFX* gfx;
};

} // namespace NuggetsInc

#endif // CLONE_NFC_STATE_H
