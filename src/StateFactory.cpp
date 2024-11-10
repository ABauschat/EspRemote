#include "StateFactory.h"
#include "MenuState.h"
#include "SnakeGameState.h"

namespace NuggetsInc {

AppState* StateFactory::createState(StateType type) {
    switch (type) {
        case MENU_STATE:
            return new MenuState();
        case SNAKE_GAME_STATE:
            return new SnakeGameState();
        // Add other cases for new states
        default:
            return nullptr;
    }
}

} // namespace NuggetsInc
