#ifndef STATE_H
#define STATE_H

#include <Arduino.h>
#include "EventManager.h"

namespace NuggetsInc {

class AppState {
public:
    virtual ~AppState() {}
    virtual void onEnter() = 0;
    virtual void onExit() = 0;
    virtual void update() = 0;
};

} // namespace NuggetsInc

#endif // STATE_H
