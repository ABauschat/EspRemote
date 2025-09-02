#include "RemoteService.h"
#include "RemoteControlState.h"
#include "DisplayUtils.h"
#include "Utils/TimeUtils.h"
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_wifi_types.h>
#include <cstring>

namespace NuggetsInc {
using namespace NuggetsInc; // For TimeUtils functions

RemoteService* RemoteService::activeInstance_ = nullptr;

RemoteService::RemoteService() : isPeerAdded_(false), selfMAC_(nullptr) {
    memset(targetMAC_, 0, sizeof(targetMAC_));
    selfMAC_ = new uint8_t[6];
    memset(selfMAC_, 0, 6);
    activeInstance_ = this;
}

bool RemoteService::begin(const uint8_t* targetMac) {
    if (!targetMac) return false;
    
    // Copy target MAC
    memcpy(targetMAC_, targetMac, 6);

    // Set sender MAC
    String selfMac = WiFi.macAddress();
    stringToMac(selfMac, selfMAC_);
    
    // Initialize WiFi in station mode
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(80);
    
    // Set WiFi channel to 1 for ESP-NOW
    WiFi.setSleep(false);
    esp_wifi_set_promiscuous(true);
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
    esp_wifi_set_promiscuous(false);
    
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
    
    struct_message message;
    memset(&message, 0, sizeof(message));
    message.messageID = (uint32_t)millis();
    
    // Set sender MAC (this Remote)
    memcpy(message.SenderMac, selfMAC_, 6);
    
    strcpy(message.messageType, "cmd");
    message.commandID = commandID;
    
    if (data) {
        strncpy(message.data, data, sizeof(message.data) - 1);
        message.data[sizeof(message.data) - 1] = '\0';
    } else {
        message.data[0] = '\0';
    }
    
    // Clear routing fields
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
    if (status != ESP_NOW_SEND_SUCCESS) {
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

    if (strcmp(message.messageType, "cmd") != 0) {
        return;
    }

    if (!isDestinationForSelf(message)) {
        return;
    }

    if (isDuplicateMessage(senderMac, message.messageID)) {
        return;
    }

    sendAck(message, senderMac);
    processDisplayCommand(message.commandID, message.data);
}

bool RemoteService::isDuplicateMessage(const uint8_t src[6], uint32_t messageID) {
    msec32 nowMs = now_ms();
    const msec32 window = 2000;
    MsgKey key;
    memcpy(key.mac, src, 6);
    key.id = messageID;
    
    auto found = recentMsgCache_.find(key);
    if (found != recentMsgCache_.end()) {
        return true;
    }
    
    if (recentMsgCache_.size() > 50) {
        auto it = recentMsgCache_.begin();
        while (it != recentMsgCache_.end()) {
            if (!within_window(it->second, window)) {
                it = recentMsgCache_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    recentMsgCache_[key] = nowMs;
    return false;
}

void RemoteService::sendAck(const struct_message& originalMsg, const uint8_t* senderMac) {
    struct_message ackMessage;
    memset(&ackMessage, 0, sizeof(ackMessage));
    
    ackMessage.messageID = originalMsg.messageID;
    strcpy(ackMessage.messageType, "ack");
    ackMessage.commandID = 0;
    ackMessage.data[0] = '\0';
    
    // Set sender MAC (this Remote)
    memcpy(ackMessage.SenderMac, selfMAC_, 6);
    
    // Clear routing fields
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

uint8_t* RemoteService::stringToMac(const String& s, uint8_t out[6]) {
    int b[6];
    if (sscanf(s.c_str(), "%02x:%02x:%02x:%02x:%02x:%02x", 
               &b[0], &b[1], &b[2], &b[3], &b[4], &b[5]) == 6) {
        for (int i = 0; i < 6; i++) {
            out[i] = (uint8_t)b[i];
        }
        return out;
    }
    return nullptr;
}

bool RemoteService::isDestinationForSelf(const struct_message& msg) {
    return isZeroMac(msg.destinationMac) || memcmp(msg.destinationMac, selfMAC_, 6) == 0;
}

bool RemoteService::isZeroMac(const uint8_t mac[6]) {
    for (int i = 0; i < 6; ++i) {
        if (mac[i] != 0) return false;
    }
    return true;
}

void RemoteService::processDisplayCommand(uint8_t commandID, const char* data) {
    auto* remoteState = RemoteControlState::getActiveInstance();

    if (!remoteState) {
        Serial.println("No active RemoteControlState to process display command");
        return;
    }

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

RemoteService::~RemoteService() {
    if (activeInstance_ == this) {
        activeInstance_ = nullptr;
    }

    esp_now_deinit();
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(50);
    
    if (selfMAC_) {
        delete[] selfMAC_;
        selfMAC_ = nullptr;
    }
}

} // namespace NuggetsInc
