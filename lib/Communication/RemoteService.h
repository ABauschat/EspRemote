#ifndef REMOTE_SERVICE_H
#define REMOTE_SERVICE_H

#include <Arduino.h>
#include <esp_now.h>
#include "MessageTypes.h"

namespace NuggetsInc {

// Simplified Remote communication service - direct sends only
class RemoteService {
public:
    RemoteService();
    ~RemoteService();

    // Initialize ESP-NOW and set target device
    bool begin(const uint8_t* targetMac);
    
    // Send command to target device (blocking with ACK wait)
    bool sendCommand(uint8_t commandID, const char* data = nullptr, uint32_t timeoutMs = 2000);
    
    // Send command without waiting for ACK
    bool sendCommandNonBlocking(uint8_t commandID, const char* data = nullptr);
    
    // Check if target peer is connected
    bool isPeerConnected() const { return isPeerAdded_; }
    
    // Get target MAC as string
    String getTargetMacString() const;

private:
    uint8_t targetMAC_[6];
    bool isPeerAdded_;
    
    // ESP-NOW callbacks
    static void onDataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void onDataRecvCallback(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
    
    // Message handling
    void processReceivedMessage(const uint8_t* senderMac, const struct_message& msg);
    void sendAck(const struct_message& originalMsg, const uint8_t* senderMac);
    
    // Display command processing
    void processDisplayCommand(uint8_t commandID, const char* data);

    // Utility functions
    static String macToString(const uint8_t mac[6]);
    static bool stringToMac(const String& s, uint8_t out[6]);

    // Static instance for callbacks
    static RemoteService* activeInstance_;
};

} // namespace NuggetsInc

#endif // REMOTE_SERVICE_H
