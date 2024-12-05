// IRRemoteState.h

#ifndef IR_REMOTE_STATE_H
#define IR_REMOTE_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <Arduino.h> 
#include "IRCommon.h"

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
        RemoteData remotes[NuggetsInc::MAX_REMOTE_SLOTS];
        uint8_t selectedSlot;
        bool slotSelected;

        void promptSlotSelection();
        void handleSlotSelection(ButtonType button);
    };

} // namespace NuggetsInc

#endif // IR_REMOTE_STATE_H
