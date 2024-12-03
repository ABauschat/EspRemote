// IRRemoteState.h
#ifndef IR_REMOTE_STATE_H
#define IR_REMOTE_STATE_H

#include "State.h"
#include "DisplayUtils.h"

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
        ButtonType mapEventTypeToButtonType(EventType eventType);
    };

} // namespace NuggetsInc

#endif // IR_REMOTE_STATE_H
