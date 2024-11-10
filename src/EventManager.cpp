#include "EventManager.h"

namespace NuggetsInc {

EventManager& EventManager::getInstance() {
    static EventManager instance;
    return instance;
}

EventManager::EventManager() {}

void EventManager::queueEvent(const Event& event) {
    eventQueue.push(event);
}

bool EventManager::getNextEvent(Event& event) {
    if (eventQueue.empty()) {
        return false;
    }
    event = eventQueue.front();
    eventQueue.pop();
    return true;
}

void EventManager::clearEvents() {
    while (!eventQueue.empty()) {
        eventQueue.pop();
    }
}

} // namespace NuggetsInc
