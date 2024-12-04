#ifndef STATEFACTORY_H
#define STATEFACTORY_H

#include "State.h"

namespace NuggetsInc {

enum StateType {
    MENU_STATE,
    SNAKE_GAME_STATE,
    CLEAR_STATE,
    CLONE_NFC_STATE,
    ENTER_REMOTE_CONTROL_STATE,
    REMOTE_CONTROL_STATE,
    SETUP_NFC_DEVICE_STATE,
    APPLICATION_STATE,
    IR_OPTIONS_STATE,
    IR_REMOTE_STATE,
    SETUP_NEW_REMOTE_STATE,
    REMOTE_BROWSER_STATE,
    NFC_OPTIONS_STATE,
    POWER_OPTIONS_STATE,
    SETTINGS_STATE,
};

class StateFactory {
public:
    static AppState* createState(StateType type);
    static AppState* createActualState(StateType type);
};

} // namespace NuggetsInc

#endif // STATEFACTORY_H
