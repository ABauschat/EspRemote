#ifndef CLEAR_STATE_H
#define CLEAR_STATE_H

#include "State.h"
#include "StateFactory.h"

namespace NuggetsInc {

    class ClearState : public AppState {
    public:
        // Constructor accepts the next state as a parameter
        explicit ClearState(StateType nextState);

        void onEnter() override;
        void update() override;
        void onExit() override;

    private:
        StateType nextState;  // Stores the next state to transition to
    };
}

#endif // CLEAR_STATE_H
