#include "StateFactory.h"
#include "MenuState.h"
#include "SnakeGameState.h"
#include "ClearState.h"
#include "CloneNFCState.h"
#include "RemoteControlState.h"
#include "EnterRemoteControlState.h"
#include "SetupNFCDeviceState.h"

namespace NuggetsInc {

AppState* StateFactory::createState(StateType type) {
    // Always return a ClearState that transitions to the desired state
    return new ClearState(type);
}

AppState* StateFactory::createActualState(StateType type) {
    switch (type) {
        case MENU_STATE:
            return new MenuState();
        case SNAKE_GAME_STATE:
            return new SnakeGameState();
        case CLONE_NFC_STATE:
            return new CloneNFCState();
        case ENTER_REMOTE_CONTROL_STATE:
            return new EnterRemoteControlState();
        case REMOTE_CONTROL_STATE:
            return new RemoteControlState(); // Default instance
        case SETUP_NFC_DEVICE_STATE:
            return new SetupNFCDeviceState();
        default:
            return nullptr;
    }
}

} // namespace NuggetsInc
