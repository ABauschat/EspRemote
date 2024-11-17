#ifndef REMOTE_CONTROL_STATE_H
#define REMOTE_CONTROL_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include "StateFactory.h"
#include "Config.h"
#include <WiFi.h>
#include <esp_now.h>

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
        RemoteControlState(uint8_t *macAddress = nullptr);  
        ~RemoteControlState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

    private:
        void handleInput(EventType eventType);
        void setupESPNow();

        static void onDataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status);
        static void onDataRecvCallback(const uint8_t *mac_addr, const uint8_t *incomingData, int len);

        void handleOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status);
        void handleOnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len);

        static RemoteControlState *activeInstance;
        DisplayUtils *displayUtils;
        uint8_t device2MAC[6];
        bool isPeerAdded;
    };
} // namespace NuggetsInc

#endif // REMOTE_CONTROL_STATE_H