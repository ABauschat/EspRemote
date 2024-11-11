#ifndef CLONE_NFC_STATE_H
#define CLONE_NFC_STATE_H

#include "State.h"
#include <Adafruit_PN532.h>

namespace NuggetsInc {

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
    void displayMessage(const String& message);
    void readNFCTag();
    String getTagType();
    String readTagData(const String& tagType);
    String readMIFAREUltralight();
    String readMIFAREClassic();
    void displayTagInfo(const String& tagType, const String& tagData);
    void handleScroll(EventType eventType);
    void splitDataIntoLines(const String& tagData);
    void NewTerminalDisplay(const String& message);
    void AddToTerminalDisplay(const String& message);

    // NFC variables
    Adafruit_PN532 nfc;
    bool tagDetected;
    uint8_t uid[7];    // Buffer to store the returned UID
    uint8_t uidLength; // Length of the UID (4 or 7 bytes)

    // Scrolling variables
    static const int MAX_DATA_LINES = 50;  // Maximum number of data lines
    String dataLines[MAX_DATA_LINES];       // Array to store data lines
    int totalDataLines;                     // Total number of data lines
    int currentScrollLine;                  // Current top line displayed
    int maxVisibleLines;                    // Number of lines visible on screen
};

} // namespace NuggetsInc

#endif // CLONE_NFC_STATE_H
