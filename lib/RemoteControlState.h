#ifndef REMOTE_CONTROL_STATE_H
#define REMOTE_CONTROL_STATE_H

#include "State.h"
#include "EventManager.h"
#include "Device.h"
#include "DisplayUtils.h"

namespace NuggetsInc {

class RemoteControlState : public AppState {
public:
    RemoteControlState();
    ~RemoteControlState() override;

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    void handleInput(EventType eventType);
    void displayRemoteControlInterface();
    DisplayUtils* displayUtils;
};

} // namespace NuggetsInc

#endif // REMOTE_CONTROL_STATE_H
