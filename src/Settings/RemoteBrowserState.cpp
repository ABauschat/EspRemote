// RemoteBrowserState.cpp
#include "RemoteBrowserState.h"
#include "Device.h"
#include "Application.h"
#include "StateFactory.h"
#include "EventManager.h"
#include "Colors.h"

namespace NuggetsInc
{
    RemoteBrowserState::RemoteBrowserState()
        : displayUtils(nullptr)
    {
    }

    RemoteBrowserState::~RemoteBrowserState()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
    }

    void RemoteBrowserState::onEnter()
    {
    }

    void RemoteBrowserState::onExit()
    {
    }

    void RemoteBrowserState::update()
    {
    }
}
