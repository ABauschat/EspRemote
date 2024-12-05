// IRRemoteState.h
#ifndef IR_REMOTE_STATE_H
#define IR_REMOTE_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <Arduino.h> 
#include <IRCommon.h>

namespace NuggetsInc
{
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
        IRData buttonIRData[BUTTON_COUNT];
        ButtonType recordingButton;       

        // Mapping function from EventType to ButtonType
        ButtonType mapEventTypeToButtonType(EventType eventType);

        // === Double Press Detection Variables ===
        unsigned long lastPressTime;               
        static const unsigned long doublePressThreshold = 500;
        uint8_t pressCount;                        

        void handleDoublePress(ButtonType button);
        bool loadIRData();
    };

} // namespace NuggetsInc

#endif // IR_REMOTE_STATE_H
