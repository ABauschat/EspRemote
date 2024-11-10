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

    Adafruit_PN532 nfc;
    bool tagDetected;
    uint8_t uid[7];    // Buffer to store the returned UID
    uint8_t uidLength; // Length of the UID (4 or 7 bytes)
};

} // namespace NuggetsInc

#endif // CLONE_NFC_STATE_H
