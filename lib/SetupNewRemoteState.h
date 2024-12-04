// SetupNewRemoteState.h
#ifndef SETUP_NEW_REMOTE_STATE_H
#define SETUP_NEW_REMOTE_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <Arduino.h> 

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
    };
}

#endif // SETUP_NEW_REMOTE_STATE_H
