#include "CloneNFCState.h"
#include "Device.h"
#include "EventManager.h"
#include "Application.h"
#include "StateFactory.h"

namespace NuggetsInc {

CloneNFCState::CloneNFCState()
    : nfc(39, 46, &Wire), tagDetected(false), uidLength(0) {}

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

    // Check for back button to return to the menu
    while (eventManager.getNextEvent(event)) {
        if (event.type == EVENT_BACK) {
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            return;
        }
    }

    if (!tagDetected) {
        readNFCTag();
    }
}

void CloneNFCState::displayMessage(const String& message) {
    Arduino_GFX* gfx = Device::getInstance().getDisplay();
    gfx->fillScreen(BLACK);
    gfx->setTextColor(WHITE);
    gfx->setTextSize(2);
    gfx->setCursor(10, 60);
    gfx->println(message);
}

void CloneNFCState::readNFCTag() {
    if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
        tagDetected = true;

        // Display UID
        Arduino_GFX* gfx = Device::getInstance().getDisplay();
        gfx->fillScreen(BLACK);
        gfx->setTextColor(WHITE);
        gfx->setTextSize(2);
        gfx->setCursor(10, 30);
        gfx->println("NFC Tag Found!");
        gfx->println("UID:");

        for (uint8_t i = 0; i < uidLength; i++) {
            if (uid[i] < 0x10) {
                gfx->print("0"); // Add leading zero for single-digit hex values
            }
            gfx->print(uid[i], HEX);
            gfx->print(" ");
        }
        gfx->println();
        gfx->println("Press Back to return");
    }
}

} // namespace NuggetsInc
