//Device Remote (wireless display)
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
            char command[20]; 
        };

    public:
        RemoteControlState();           
        ~RemoteControlState() override;

        void onEnter() override; 
        void onExit() override; 
        void update() override;  

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

        DisplayUtils *displayUtils; 
        NFCLogic *nfcLogic;        

        uint8_t device2MAC[6];
        bool isPeerAdded;    
        bool tagDetected;

        TagData *currentTagData;
        static RemoteControlState *activeInstance;
    };

} // namespace NuggetsInc

#endif // REMOTE_CONTROL_STATE_H
