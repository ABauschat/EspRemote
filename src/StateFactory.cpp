// StateFactory.cpp
#include "StateFactory.h"
#include "MenuState.h"
#include "SnakeGameState.h"
#include "ClearState.h"

namespace NuggetsInc {

AppState* StateFactory::createState(StateType type) {
    // Always return a ClearState that transitions to the desired state
    return new ClearState(type);
}

AppState *StateFactory::createActualState(StateType type)
{
    switch (type) {
        case MENU_STATE:
            return new MenuState();
        case SNAKE_GAME_STATE:
            return new SnakeGameState();
        default:
            return nullptr;
    }
    return nullptr;
}

} // namespace NuggetsInc
