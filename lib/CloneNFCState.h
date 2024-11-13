// CloneNFCState.h
#ifndef CLONE_NFC_STATE_H
#define CLONE_NFC_STATE_H

#include "State.h"
#include <vector>
#include <String.h>
#include "NFCLogic.h"
#include "DisplayUtils.h"
#include "Colors.h"

namespace NuggetsInc {

class CloneNFCState : public AppState {
public:
    CloneNFCState();
    ~CloneNFCState();

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    // NFC functions
    void readNFCTag();
    bool cloneTagData();

    // Utility functions
    void handleScroll(EventType eventType);
    void splitDataIntoLines(const String& tagData);
    void resetNFC();

    // Variables
    DisplayTab currentTab;
    bool tagDetected;
    std::vector<String> dataLines; // Arduino's String
    int currentScrollLine;
    int maxVisibleLines;
    std::vector<NDEFRecord> parsedRecords;
    String availableSpace;

    // NFC variables
    NFCLogic* nfcLogic;
    uint8_t uid[7];
    uint8_t uidLength;
    String clonedTagType;
    String clonedData;

    // Display
    DisplayUtils* displayUtils;
};

} // namespace NuggetsInc

#endif // CLONE_NFC_STATE_H
