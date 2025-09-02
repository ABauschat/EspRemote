// RemoteControlState.h
#ifndef REMOTE_CONTROL_STATE_H
#define REMOTE_CONTROL_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include "StateFactory.h"
#include "Config.h"
#include "Communication/MessageTypes.h"

namespace NuggetsInc {
    class RemoteService; // Forward declaration
}

namespace NuggetsInc
{
    class RemoteControlState : public AppState
    {
    public:
        // Define command types
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
            BEGIN_PLOT,
            PLOT_POINT,
            UNKNOWN
        };

        RemoteControlState(uint8_t *macAddress = nullptr);
        ~RemoteControlState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

        // Public display command handlers for RemoteService
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
        void handleBeginPlot(const char* data);
        void handlePlotPoint(const char* data);
        void handleSyncNodes(const char* data);

        // Get active instance for RemoteService
        static RemoteControlState* getActiveInstance() { return activeInstance; }

    private:
        void handleInput(EventType eventType);

        // Private members

        static RemoteControlState *activeInstance;
        DisplayUtils *displayUtils;
        RemoteService *remoteService_;
        uint8_t device2MAC[6];
    };
} // namespace NuggetsInc

#endif // REMOTE_CONTROL_STATE_H
