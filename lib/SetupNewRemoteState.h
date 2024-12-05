// SetupNewRemoteState.h

#ifndef SETUP_NEW_REMOTE_STATE_H
#define SETUP_NEW_REMOTE_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <Arduino.h> 
#include "IRCommon.h"

namespace NuggetsInc
{
    class SetupNewRemoteState : public AppState
    {
    public:
        SetupNewRemoteState();
        ~SetupNewRemoteState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

    private:
        DisplayUtils* displayUtils;
        RemoteData remotes[NuggetsInc::MAX_REMOTE_SLOTS];
        uint8_t selectedSlot;
        bool slotSelected;
        IRData buttonIRData[BUTTON_COUNT];
        ButtonType recordingButton;       
        unsigned long lastPressTime;               
        static const unsigned long doublePressThreshold = 500;
        uint8_t pressCount;                        

        void handleDoublePress(ButtonType button);
        void promptSlotSelection();
        void handleSlotSelection(ButtonType button);
    };
}

#endif // SETUP_NEW_REMOTE_STATE_H
