#include "StateFactory.h"
#include "MenuState.h"
#include "SnakeGameState.h"
#include "ClearState.h"

namespace NuggetsInc {

AppState* StateFactory::createState(StateType type) {
    switch (type) {
        case MENU_STATE:
            return new MenuState();
        case SNAKE_GAME_STATE:
            return new SnakeGameState();
        case CLEAR_STATE:
             return new ClearState(type); 
        default:
            return nullptr;
    }
}

// Create ClearState with specified next state
AppState* StateFactory::createClearStateWithNext(StateType nextState) {
    return new ClearState(nextState);
}

} // namespace NuggetsInc
