#ifndef EVENTMANAGER_H
#define EVENTMANAGER_H

#include <queue>
#include <functional>

namespace NuggetsInc {

enum EventType {
    EVENT_NONE,
    EVENT_UP,
    EVENT_DOWN,
    EVENT_LEFT,
    EVENT_RIGHT,
    EVENT_SELECT,
    EVENT_BACK,
    // Add other event types as needed
};

struct Event {
    EventType type;
    // Additional event data can be added here
};

class EventManager {
public:
    static EventManager& getInstance();

    // Prevent copying
    EventManager(const EventManager&) = delete;
    EventManager& operator=(const EventManager&) = delete;

    void queueEvent(const Event& event);
    bool getNextEvent(Event& event);
    void clearEvents();

private:
    EventManager();
    std::queue<Event> eventQueue;
};

} // namespace NuggetsInc

#endif // EVENTMANAGER_H
