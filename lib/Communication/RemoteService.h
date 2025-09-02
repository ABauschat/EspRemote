#ifndef REMOTE_SERVICE_H
#define REMOTE_SERVICE_H

#include <Arduino.h>
#include <esp_now.h>
#include <map>
#include "MessageTypes.h"
#include "Utils/TimeUtils.h"

namespace NuggetsInc {

// Simplified Remote communication service - direct sends only
class RemoteService {
public:
    RemoteService();
    ~RemoteService();

    // Initialize ESP-NOW and set target device
    bool begin(const uint8_t* targetMac);
    bool sendCommand(uint8_t commandID, const char* data = nullptr, uint32_t timeoutMs = 2000);
    bool sendCommandNonBlocking(uint8_t commandID, const char* data = nullptr);
    bool isPeerConnected() const { return isPeerAdded_; }
    String getTargetMacString() const;

private:
    uint8_t targetMAC_[6];
    uint8_t* selfMAC_;
    bool isPeerAdded_;
    
    // ESP-NOW callbacks
    static void onDataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
    static void onDataRecvCallback(const uint8_t *mac_addr, const uint8_t *incomingData, int len);
    
    // Message handling
    void processReceivedMessage(const uint8_t* senderMac, const struct_message& msg);
    bool isDuplicateMessage(const uint8_t src[6], uint32_t messageID);
    void sendAck(const struct_message& originalMsg, const uint8_t* senderMac);
    bool isDestinationForSelf(const struct_message& msg);
    
    // Display command processing
    void processDisplayCommand(uint8_t commandID, const char* data);

    // Utility functions
    static String macToString(const uint8_t mac[6]);
    static uint8_t* stringToMac(const String& s, uint8_t out[6]);
    bool isZeroMac(const uint8_t mac[6]);

    // Deduplication cache
    struct MsgKey {
        uint8_t mac[6];
        uint32_t id;
    };
    struct MsgKeyCmp {
        bool operator()(const MsgKey& a, const MsgKey& b) const {
            int c = memcmp(a.mac, b.mac, 6);
            return c < 0 || (c == 0 && a.id < b.id);
        }
    };
    std::map<MsgKey, msec32, MsgKeyCmp> recentMsgCache_;

    // Static instance for callbacks
    static RemoteService* activeInstance_;
};

} // namespace NuggetsInc

#endif // REMOTE_SERVICE_H
