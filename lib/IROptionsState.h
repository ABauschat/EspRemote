#ifndef IR_OPTIONS_STATE_H
#define IR_OPTIONS_STATE_H

#include "State.h"

namespace NuggetsInc
{

    class IROptionsState : public AppState
    {
    public:
        IROptionsState();
        ~IROptionsState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;
    };

} // namespace NuggetsInc

#endif // IR_OPTIONS_STATE_H
