#include "RemoteService.h"
#include "RemoteControlState.h"
#include "DisplayUtils.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <cstring>

namespace NuggetsInc {

RemoteService* RemoteService::activeInstance_ = nullptr;

RemoteService::RemoteService() : isPeerAdded_(false) {
    memset(targetMAC_, 0, sizeof(targetMAC_));
    activeInstance_ = this;
}

RemoteService::~RemoteService() {
    if (activeInstance_ == this) {
        activeInstance_ = nullptr;
    }
}

bool RemoteService::begin(const uint8_t* targetMac) {
    if (!targetMac) return false;
    
    // Copy target MAC
    memcpy(targetMAC_, targetMac, 6);
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    // Set WiFi channel to 1 for ESP-NOW
    WiFi.setSleep(false);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    
    Serial.print("Remote MAC Address: ");
    Serial.println(WiFi.macAddress());
    Serial.print("Target MAC Address: ");
    Serial.println(macToString(targetMAC_));
    
    // Initialize ESP-NOW
    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW initialization failed");
        return false;
    }
    
    // Register callbacks
    esp_now_register_send_cb(onDataSentCallback);
    esp_now_register_recv_cb(onDataRecvCallback);
    
    // Add target as peer
    if (!esp_now_is_peer_exist(targetMAC_)) {
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, targetMAC_, 6);
        peerInfo.channel = 1;
        peerInfo.encrypt = false;
        
        if (esp_now_add_peer(&peerInfo) == ESP_OK) {
            isPeerAdded_ = true;
            Serial.println("Target peer added successfully");
        } else {
            Serial.println("Failed to add target peer");
            return false;
        }
    } else {
        isPeerAdded_ = true;
        Serial.println("Target peer already exists");
    }

    return true;
}

bool RemoteService::sendCommand(uint8_t commandID, const char* data, uint32_t timeoutMs) {
    if (!isPeerAdded_) {
        Serial.println("Cannot send: Target peer not connected");
        return false;
    }
    
    // Build message
    struct_message message;
    memset(&message, 0, sizeof(message));
    message.messageID = millis();
    
    // Set sender MAC
    String selfMac = WiFi.macAddress();
    int b[6];
    if (sscanf(selfMac.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", 
               &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            message.SenderMac[i] = (uint8_t)b[i];
        }
    }
    
    strcpy(message.messageType, "cmd");
    message.commandID = commandID;
    
    if (data) {
        strncpy(message.data, data, sizeof(message.data) - 1);
        message.data[sizeof(message.data) - 1] = '\0';
    } else {
        message.data[0] = '\0';
    }
    
    // Clear routing fields (direct send only)
    memset(message.destinationMac, 0, sizeof(message.destinationMac));
    message.path[0] = '\0';
    
    // Send message
    esp_err_t result = esp_now_send(targetMAC_, (uint8_t*)&message, sizeof(message));
    if (result == ESP_OK) {
        return true;
    } else {
        Serial.printf("Failed to send command: %s\n", esp_err_to_name(result));
        return false;
    }
}

bool RemoteService::sendCommandNonBlocking(uint8_t commandID, const char* data) {
    return sendCommand(commandID, data, 0); 
}

String RemoteService::getTargetMacString() const {
    return macToString(targetMAC_);
}

void RemoteService::onDataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
    } else {
        Serial.println("Message send failed");
    }
}

void RemoteService::onDataRecvCallback(const uint8_t *mac_addr, const uint8_t *incomingData, int len) {
    if (activeInstance_ && len >= sizeof(struct_message)) {
        struct_message receivedMessage;
        memcpy(&receivedMessage, incomingData, sizeof(struct_message));
        activeInstance_->processReceivedMessage(mac_addr, receivedMessage);
    }
}

void RemoteService::processReceivedMessage(const uint8_t* senderMac, const struct_message& msg) {
    // Null-terminate strings for safety
    struct_message message = msg;
    message.messageType[sizeof(message.messageType) - 1] = '\0';
    message.data[sizeof(message.data) - 1] = '\0';
    message.path[sizeof(message.path) - 1] = '\0';
    
    // Handle command messages (display updates from Sender)
    if (strcmp(message.messageType, "cmd") == 0) {
        // Send ACK back
        sendAck(message, senderMac);
        
        // Process display command - delegate to DisplayUtils
        processDisplayCommand(message.commandID, message.data);
    }
}

void RemoteService::sendAck(const struct_message& originalMsg, const uint8_t* senderMac) {
    struct_message ackMessage;
    memset(&ackMessage, 0, sizeof(ackMessage));
    
    ackMessage.messageID = originalMsg.messageID;
    strcpy(ackMessage.messageType, "ack");
    ackMessage.commandID = 0;
    ackMessage.data[0] = '\0';
    
    // Set sender MAC (this Remote)
    String selfMac = WiFi.macAddress();
    int b[6];
    if (sscanf(selfMac.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", 
               &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            ackMessage.SenderMac[i] = (uint8_t)b[i];
        }
    }
    
    // Clear routing fields (direct ACK)
    memset(ackMessage.destinationMac, 0, sizeof(ackMessage.destinationMac));
    ackMessage.path[0] = '\0';
    
    // Send ACK back to sender
    esp_err_t result = esp_now_send(senderMac, (uint8_t*)&ackMessage, sizeof(ackMessage));
    if (result == ESP_OK) {
    } else {
        Serial.printf("Failed to send ACK: %s\n", esp_err_to_name(result));
    }
}

String RemoteService::macToString(const uint8_t mac[6]) {
    char buf[18];
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X", 
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    return String(buf);
}

bool RemoteService::stringToMac(const String& s, uint8_t out[6]) {
    int b[6];
    if (sscanf(s.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", 
               &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            out[i] = (uint8_t)b[i];
        }
        return true;
    }
    return false;
}

void RemoteService::processDisplayCommand(uint8_t commandID, const char* data) {
    Serial.printf("Processing display command 0x%02X with data: %s\n", commandID, data ? data : "");

    // Get the active RemoteControlState instance to process display commands
    auto* remoteState = RemoteControlState::getActiveInstance();

    if (!remoteState) {
        Serial.println("No active RemoteControlState to process display command");
        return;
    }

    // Process the display command using the existing handlers
    switch (commandID) {
        case CMD_CLEAR_DISPLAY:
            remoteState->handleClearDisplay();
            break;
        case CMD_DISPLAY_MESSAGE:
            if (data && strlen(data) > 0)
                remoteState->handleDisplayMessage(String(data));
            break;
        case CMD_NEW_TERMINAL_DISPLAY:
            if (data && strlen(data) > 0)
                remoteState->handleNewTerminalDisplay(String(data));
            break;
        case CMD_ADD_TO_TERMINAL:
            if (data && strlen(data) > 0)
                remoteState->handleAddToTerminalDisplay(String(data));
            break;
        case CMD_PRINTLN:
            if (data && strlen(data) > 0)
                remoteState->handlePrintln(String(data));
            break;
        case CMD_PRINT:
            if (data && strlen(data) > 0)
                remoteState->handlePrint(String(data));
            break;
        case CMD_SET_CURSOR:
            remoteState->handleSetCursor(data);
            break;
        case CMD_SET_TEXT_SIZE:
            remoteState->handleSetTextSize(data);
            break;
        case CMD_SET_TEXT_COLOR:
            remoteState->handleSetTextColor(data);
            break;
        case CMD_FILL_SCREEN:
            remoteState->handleFillScreen(data);
            break;
        case CMD_DRAW_RECT:
            remoteState->handleDrawRect(data);
            break;
        case CMD_FILL_RECT:
            remoteState->handleFillRect(data);
            break;
        case CMD_BEGIN_PLOT:
            remoteState->handleBeginPlot(data);
            break;
        case CMD_PLOT_POINT:
            remoteState->handlePlotPoint(data);
            break;
        case CMD_SYNC_NODES:
            remoteState->handleSyncNodes(data);
            break;
        default:
            Serial.printf("Unknown display command ID: 0x%02X\n", commandID);
            break;
    }
}

} // namespace NuggetsInc
