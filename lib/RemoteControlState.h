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
            char data[50];
        };

        enum class CommandType
        {
            CLEAR_DISPLAY,
            DISPLAY_MESSAGE,
            NEW_TERMINAL_DISPLAY,
            ADD_TO_TERMINAL_DISPLAY,
            PRINTLN,
            PRINT,
            SET_CURSOR,
            SET_TEXT_SIZE,
            SET_TEXT_COLOR,
            FILL_SCREEN,
            DRAW_RECT,
            FILL_RECT,
            UNKNOWN
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

        CommandType mapCommandStringToEnum(const char* command);

        void handleClearDisplay();
        void handleDisplayMessage(const String& message);
        void handleNewTerminalDisplay(const String& message);
        void handleAddToTerminalDisplay(const String& message);
        void handlePrintln(const String& message);
        void handlePrint(const String& message);
        void handleSetCursor(const char* data);
        void handleSetTextSize(const char* data);
        void handleSetTextColor(const char* data);
        void handleFillScreen(const char* data);
        void handleDrawRect(const char* data);
        void handleFillRect(const char* data);

        static RemoteControlState *activeInstance;
        DisplayUtils *displayUtils;
        uint8_t device2MAC[6];
        bool isPeerAdded;
    };
} // namespace NuggetsInc

#endif // REMOTE_CONTROL_STATE_H
