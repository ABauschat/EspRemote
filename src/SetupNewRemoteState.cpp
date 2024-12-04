// SetupNewRemoteState.cpp
#include "SetupNewRemoteState.h"
#include "Device.h"
#include "Application.h"
#include "StateFactory.h"
#include "EventManager.h"
#include "Colors.h"

namespace NuggetsInc
{
    SetupNewRemoteState::SetupNewRemoteState()
        : displayUtils(nullptr)
    {
    }

    SetupNewRemoteState::~SetupNewRemoteState()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
    }

    void SetupNewRemoteState::onEnter()
    {
    }

    void SetupNewRemoteState::onExit()
    {
    }

    void SetupNewRemoteState::update()
    {
    }
}
