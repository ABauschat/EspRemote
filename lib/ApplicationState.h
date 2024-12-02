#ifndef APPLICATION_STATE_H
#define APPLICATION_STATE_H

#include "State.h"
#include "DisplayUtils.h"

namespace NuggetsInc
{

    class ApplicationState : public AppState
    {
    public:
        ApplicationState();
        ~ApplicationState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

    private:
        void displayMenu();
        void executeSelection();

        static const int menuItems = 1; // Initially, only Snake Game
        String menu[menuItems];
        int menuIndex;

        DisplayUtils *displayUtils;
    };

} // namespace NuggetsInc

#endif // APPLICATION_STATE_H
