#include "StateFactory.h"
#include "Settings/MenuState.h"
#include "Games/SnakeGameState.h"
#include "Utils/ClearState.h"
#include "NFC/CloneNFCState.h"
#include "Communication/RemoteControlState.h"
#include "Communication/EnterRemoteControlState.h"
#include "NFC/SetupNFCDeviceState.h"
#include "Games/ApplicationState.h"
#include "IR/IROptionsState.h"
#include "IR/IRRemoteState.h"
#include "NFC/NFCOptionsState.h"
#include "Settings/PowerOptionsState.h"
#include "Settings/SettingsState.h"
#include "Settings/SetupNewRemoteState.h"
#include "Settings/RemoteBrowserState.h"
#include "Communication/MacAddressMenuState.h"
#include "Communication/SyncNodesState.h"

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
        case MAC_ADDRESS_MENU_STATE:
            return new MacAddressMenuState();
        case SYNC_NODES_STATE:
            return new SyncNodesState();
        default:
            return nullptr;
    }
}

} // namespace NuggetsInc
