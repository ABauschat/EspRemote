#ifndef IR_OPTIONS_STATE_H
#define IR_OPTIONS_STATE_H

#include "State.h"
#include "DisplayUtils.h"

namespace NuggetsInc {

class IROptionsState : public AppState {
public:
    IROptionsState();
    ~IROptionsState() override;

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    void displayMenu();
    void executeSelection();

    static const int menuItems = 2; // Two options: Remote Browser and Setup New Remote
    String menu[menuItems];
    int menuIndex;

    DisplayUtils* displayUtils;
};

} // namespace NuggetsInc

#endif // IR_OPTIONS_STATE_H
