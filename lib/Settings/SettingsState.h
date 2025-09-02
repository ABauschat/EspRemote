#ifndef SETTINGS_STATE_H
#define SETTINGS_STATE_H

#include "State.h"

namespace NuggetsInc
{

    class SettingsState : public AppState
    {
    public:
        SettingsState();
        ~SettingsState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;
    };

} // namespace NuggetsInc

#endif // SETTINGS_STATE_H
