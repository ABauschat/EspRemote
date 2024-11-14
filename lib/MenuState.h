// MenuState.h
#ifndef MENUSTATE_H
#define MENUSTATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <vector>
#include <string>

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

    DisplayUtils* displayUtils;
};

} // namespace NuggetsInc

#endif // MENUSTATE_H
