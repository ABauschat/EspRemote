#ifndef POWER_OPTIONS_STATE_H
#define POWER_OPTIONS_STATE_H

#include "State.h"

#define PIN_BAT_VOLT 4

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

    private:
        float calculateBatteryPercentage(float voltage); // Declare the function here
        unsigned long lastUpdateTime;
    };

} // namespace NuggetsInc

#endif // POWER_OPTIONS_STATE_H
