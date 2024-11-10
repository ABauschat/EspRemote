#include "ClearState.h"
#include "Device.h"
#include "Application.h"

namespace NuggetsInc {

ClearState::ClearState(StateType nextState) : nextState(nextState) {}

void ClearState::onEnter() {
    Device::getInstance().getDisplay()->fillScreen(BLACK);
    delay(100);  // Brief delay to ensure screen is cleared
}

void ClearState::update() {
    // Transition to the specified next state
    Application::getInstance().changeState(StateFactory::createState(nextState));
}

void ClearState::onExit() {
    // No cleanup needed for now, but can add if needed later
}

} // namespace NuggetsInc
