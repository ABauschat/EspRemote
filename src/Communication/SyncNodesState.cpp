#include "SyncNodesState.h"
#include "Device.h"
#include "StateFactory.h"
#include "Application.h"
#include "EventManager.h"
#include "Colors.h"
#include "MacAddressStorage.h"
#include "MessageTypes.h"

namespace NuggetsInc {

SyncNodesState* SyncNodesState::activeInstance = nullptr;

SyncNodesState::SyncNodesState()
    : displayUtils(nullptr), currentBroadcastIndex(0), lastBroadcastTime(0),
      broadcastStartTime(0), broadcastInProgress(false), broadcastComplete(false) {
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
}

SyncNodesState::~SyncNodesState() {
    delete displayUtils;
}

void SyncNodesState::onEnter() {
    activeInstance = this;

    // Initialize ESP-NOW if not already initialized
    WiFi.mode(WIFI_STA);

    // Check if ESP-NOW is already initialized, if not, initialize it
    esp_err_t initResult = esp_now_init();
    if (initResult == ESP_OK) {
        Serial.println("ESP-NOW initialized successfully for sync");
    } else if (initResult == ESP_ERR_ESPNOW_INTERNAL) {
        Serial.println("ESP-NOW already initialized, reusing existing system");
    } else {
        Serial.println("ESP-NOW initialization failed");
        displayUtils->displayMessage("ESP-NOW init failed");
        return;
    }

    loadMacAddresses();
    updateDisplay();
}

void SyncNodesState::onExit() {
    activeInstance = nullptr;
}

void SyncNodesState::update() {
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    while (eventManager.getNextEvent(event)) {
        switch (event.type) {
            case EVENT_SELECT:
            case EVENT_ACTION_ONE:
                if (!broadcastInProgress && !broadcastComplete) {
                    startBroadcast();
                }
                break;
            case EVENT_BACK:
            case EVENT_ACTION_TWO:
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            default:
                break;
        }
    }
    
    if (broadcastInProgress) {
        unsigned long currentTime = millis();

        if ((unsigned long)(currentTime - broadcastStartTime) > BROADCAST_TIMEOUT) {
            broadcastInProgress = false;
            broadcastComplete = true;
            updateDisplay();
            return;
        }

        if ((unsigned long)(currentTime - lastBroadcastTime) > BROADCAST_INTERVAL) {
            broadcastToNextNode();
        }
    }
}

void SyncNodesState::loadMacAddresses() {
    MacAddressStorage& macStorage = MacAddressStorage::getInstance();
    macAddresses = macStorage.getAllMacAddresses();
}

void SyncNodesState::startBroadcast() {
    if (macAddresses.empty()) {
        displayUtils->displayMessage("No MAC addresses to sync");
        return;
    }
    
    broadcastInProgress = true;
    broadcastComplete = false;
    currentBroadcastIndex = 0;
    broadcastStartTime = millis();
    lastBroadcastTime = 0; // Force immediate first broadcast
    
    updateDisplay();
}

void SyncNodesState::broadcastToNextNode() {
    if (currentBroadcastIndex >= macAddresses.size()) {
        // Finished broadcasting to all nodes
        broadcastInProgress = false;
        broadcastComplete = true;
        updateDisplay();
        return;
    }
    
    String targetMac = macAddresses[currentBroadcastIndex];
    sendSyncCommand(targetMac);
    
    currentBroadcastIndex++;
    lastBroadcastTime = millis();
    updateDisplay();
}

void SyncNodesState::sendSyncCommand(const String& targetMac) {
    // Convert MAC string to byte array
    uint8_t macBytes[6];
    sscanf(targetMac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &macBytes[0], &macBytes[1], &macBytes[2],
           &macBytes[3], &macBytes[4], &macBytes[5]);

    // Check if ESP-NOW is initialized
    if (!esp_now_is_peer_exist(macBytes)) {
        // Add peer if not already added
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, macBytes, 6);
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        if (esp_now_add_peer(&peerInfo) != ESP_OK) {
            Serial.println("Failed to add peer for sync");
            return;
        }
    }

    // Create sync message with all MAC addresses
    struct_message syncMessage;
    memset(&syncMessage, 0, sizeof(syncMessage)); // Clear all fields
    syncMessage.messageID = millis();
    strcpy(syncMessage.messageType, "cmd");
    syncMessage.commandID = CMD_SYNC_NODES;

    // Pack MAC addresses into data field (comma-separated)
    String macList = "";
    for (int i = 0; i < macAddresses.size() && i < 5; i++) { // Limit to 5 MACs due to data size
        if (i > 0) macList += ",";
        macList += macAddresses[i];
    }
    strncpy(syncMessage.data, macList.c_str(), sizeof(syncMessage.data) - 1);
    syncMessage.data[sizeof(syncMessage.data) - 1] = '\0';

    // Set sender MAC
    String selfMacStr = WiFi.macAddress();
    selfMacStr.toUpperCase();
    int b[6];
    sscanf(selfMacStr.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]);
    for (int i = 0; i < 6; i++) syncMessage.SenderMac[i] = (uint8_t)b[i];

    // Set destination MAC to the target device
    memcpy(syncMessage.destinationMac, macBytes, 6);

    // Initialize path as empty for direct send
    syncMessage.path[0] = '\0';

    // Send the message
    esp_err_t result = esp_now_send(macBytes, (uint8_t*)&syncMessage, sizeof(syncMessage));

    Serial.print("Sending sync to: ");
    Serial.print(targetMac);
    Serial.print(" Result: ");
    Serial.println(result == ESP_OK ? "OK" : "FAIL");
}

void SyncNodesState::updateDisplay() {
    displayUtils->clearDisplay();
    
    // Title
    displayUtils->setTextColor(COLOR_WHITE);
    displayUtils->setTextSize(2);
    displayUtils->setCursor(10, 10);
    displayUtils->print("Sync Nodes");
    
    // Status
    displayUtils->setTextSize(1);
    displayUtils->setCursor(10, 40);
    
    if (macAddresses.empty()) {
        displayUtils->print("No MAC addresses to sync");
    } else if (!broadcastInProgress && !broadcastComplete) {
        displayUtils->print("Ready to sync " + String(macAddresses.size()) + " nodes");
        displayUtils->setCursor(10, 60);
        displayUtils->print("Press SELECT to start");
    } else if (broadcastInProgress) {
        displayUtils->print("Syncing... (" + String(currentBroadcastIndex) + "/" + String(macAddresses.size()) + ")");
        if (currentBroadcastIndex > 0) {
            displayUtils->setCursor(10, 60);
            displayUtils->print("Current: " + macAddresses[currentBroadcastIndex - 1]);
        }
    } else if (broadcastComplete) {
        displayUtils->print("Sync complete!");
        displayUtils->setCursor(10, 60);
        displayUtils->print("Sent to " + String(macAddresses.size()) + " nodes");
    }
    
    // Instructions
    displayUtils->setCursor(10, 200);
    displayUtils->print("BACK: Return to menu");
}

} // namespace NuggetsInc
