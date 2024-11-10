#ifndef MENUSTATE_H
#define MENUSTATE_H

#include "State.h"

namespace NuggetsInc {

class MenuState : public AppState {
public:
    MenuState();
    ~MenuState();

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    void displayMenu();
    void executeSelection();

    static const int menuItems = 4;
    String menu[menuItems];
    int menuIndex;
};

} // namespace NuggetsInc

#endif // MENUSTATE_H
