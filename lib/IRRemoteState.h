#ifndef IR_REMOTE_STATE_H
#define IR_REMOTE_STATE_H

#include "State.h"

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
    };

} // namespace NuggetsInc

#endif // IR_REMOTE_STATE_H
