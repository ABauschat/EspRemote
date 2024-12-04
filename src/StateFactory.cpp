#include "StateFactory.h"
#include "MenuState.h"
#include "SnakeGameState.h"
#include "ClearState.h"
#include "CloneNFCState.h"
#include "RemoteControlState.h"
#include "EnterRemoteControlState.h"
#include "SetupNFCDeviceState.h"
#include "ApplicationState.h"
#include "IROptionsState.h"
#include "IRRemoteState.h"
#include "NFCOptionsState.h"
#include "PowerOptionsState.h"
#include "SettingsState.h"
#include "SetupNewRemoteState.h"
#include "RemoteBrowserState.h"

namespace NuggetsInc {

AppState* StateFactory::createState(StateType type) {
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
            return new RemoteControlState();
        case SETUP_NFC_DEVICE_STATE:
            return new SetupNFCDeviceState();
        case APPLICATION_STATE:
            return new ApplicationState();
        case IR_OPTIONS_STATE:
            return new IROptionsState();
        case IR_REMOTE_STATE:
            return new IRRemoteState();
        case NFC_OPTIONS_STATE:
            return new NFCOptionsState();
        case POWER_OPTIONS_STATE:
            return new PowerOptionsState();
        case SETTINGS_STATE:
            return new SettingsState();
        case SETUP_NEW_REMOTE_STATE:
            return new SetupNewRemoteState();
        case REMOTE_BROWSER_STATE:
            return new RemoteBrowserState();
        default:
            return nullptr;
    }
}

} // namespace NuggetsInc
