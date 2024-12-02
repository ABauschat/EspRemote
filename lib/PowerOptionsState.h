#ifndef POWER_OPTIONS_STATE_H
#define POWER_OPTIONS_STATE_H

#include "State.h"

namespace NuggetsInc
{

    class PowerOptionsState : public AppState
    {
    public:
        PowerOptionsState();
        ~PowerOptionsState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;
    };

} // namespace NuggetsInc

#endif // POWER_OPTIONS_STATE_H
