#ifndef ENTER_REMOTE_CONTROL_STATE_H
#define ENTER_REMOTE_CONTROL_STATE_H

#include "State.h"
#include "EventManager.h"
#include "Device.h"
#include "DisplayUtils.h"
#include "NFCLogic.h"

namespace NuggetsInc
{
    class EnterRemoteControlState : public AppState
    {
    public:
        EnterRemoteControlState();
        ~EnterRemoteControlState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

    private:
        void readNFCTag();

        DisplayUtils *displayUtils;
        NFCLogic *nfcLogic;
        uint8_t device2MAC[6];
        TagData *currentTagData;

        static EnterRemoteControlState *activeInstance;
    };
} // namespace NuggetsInc

#endif // ENTER_REMOTE_CONTROL_STATE_H
