// RemoteBrowserState.h
#ifndef REMOTE_BROWSER_STATE_H
#define REMOTE_BROWSER_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <Arduino.h> 

namespace NuggetsInc
{
    class RemoteBrowserState : public AppState
    {
    public:
        RemoteBrowserState();
        ~RemoteBrowserState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

    private:
        DisplayUtils* displayUtils;
    };
}

#endif // REMOTE_BROWSER_STATE_H
