#ifndef REMOTE_CONTROL_STATE_H
#define REMOTE_CONTROL_STATE_H

#include "State.h"
#include "EventManager.h"
#include "Device.h"
#include "DisplayUtils.h"
#include "NFCLogic.h"
#include <esp_now.h>
#include <WiFi.h>

namespace NuggetsInc
{
    class RemoteControlState : public AppState
    {
        struct struct_message
        {
            char messageType[10];
            char command[20]; // Increased to 20 bytes
        };

    public:
        RemoteControlState();           // Constructor
        ~RemoteControlState() override; // Destructor

        void onEnter() override; // Called when entering this state
        void onExit() override;  // Called when exiting this state
        void update() override;  // Called during update loop

    private:
        void handleInput(EventType eventType);
        void displayRemoteControlInterface();
        void setupESPNow();
        void readNFCTag();

            // Static callbacks required by ESP-NOW
            static void onDataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
        static void onDataRecvCallback(const uint8_t *mac_addr, const uint8_t *incomingData, int len);

        // Instance methods to handle callbacks
        void handleOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
        void handleOnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len);

        DisplayUtils *displayUtils; // Display utility
        NFCLogic *nfcLogic;         // NFC logic

        uint8_t device2MAC[6]; // MAC address of Device 2
        bool isPeerAdded;      // Flag to indicate if Device 2 is added as a peer
        bool tagDetected;

        TagData *currentTagData;
        static RemoteControlState *activeInstance; // Pointer to the active instance
    };

} // namespace NuggetsInc

#endif // REMOTE_CONTROL_STATE_H
