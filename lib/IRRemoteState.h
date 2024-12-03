// IRRemoteState.h
#ifndef IR_REMOTE_STATE_H
#define IR_REMOTE_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <Arduino.h> // For unsigned long and other Arduino-specific types

namespace NuggetsInc
{
    const uint8_t MAX_RAW_DATA_SIZE = 100; // Maximum size for raw data storage

    // Enum to represent buttons
    enum ButtonType
    {
        BUTTON_ACTION_ONE,
        BUTTON_ACTION_TWO,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_SELECT,
        BUTTON_BACK,
        BUTTON_COUNT // Total number of buttons
    };

    struct IRData
    {
        uint16_t rawData[MAX_RAW_DATA_SIZE];
        uint8_t rawDataLength;
        bool isValid;
    };

    class IRRemoteState : public AppState
    {
    public:
        IRRemoteState();
        ~IRRemoteState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

    private:
        DisplayUtils *displayUtils;
        IRData buttonIRData[BUTTON_COUNT]; // Array to store IR data for each button
        ButtonType recordingButton;        // Current button being recorded

        // Mapping function from EventType to ButtonType
        ButtonType mapEventTypeToButtonType(EventType eventType);

        // === Double Press Detection Variables ===
        unsigned long lastPressTime;                // Timestamp of the last button press
        static const unsigned long doublePressThreshold = 500; // Time threshold for double press in milliseconds
        uint8_t pressCount;                         // Counter for button presses

        // === Methods for Handling Double Press and Flash Operations ===

        /**
         * @brief Handles the action when a double press is detected on a specific button.
         * 
         * @param button The ButtonType that was double-pressed.
         */
        void handleDoublePress(ButtonType button);

        /**
         * @brief Loads IR data from flash memory into the buttonIRData array.
         * 
         * @return true if data was successfully loaded.
         * @return false if loading failed.
         */
        bool loadIRData();
    };

} // namespace NuggetsInc

#endif // IR_REMOTE_STATE_H
