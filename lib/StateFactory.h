#ifndef STATEFACTORY_H
#define STATEFACTORY_H

#include "State.h"

namespace NuggetsInc {

enum StateType {
    MENU_STATE,
    SNAKE_GAME_STATE
    // Add other states as needed
};

class StateFactory {
public:
    static AppState* createState(StateType type);
};

} // namespace NuggetsInc

#endif // STATEFACTORY_H
