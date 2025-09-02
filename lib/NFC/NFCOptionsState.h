#ifndef NFC_OPTIONS_STATE_H
#define NFC_OPTIONS_STATE_H

#include "State.h"
#include "DisplayUtils.h"

namespace NuggetsInc {

class NFCOptionsState : public AppState {
public:
    NFCOptionsState();
    ~NFCOptionsState() override;

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    void displayMenu();
    void executeSelection();

    static const int menuItems = 2; // Two options: Setup NFC Device and Clone NFC Chip
    String menu[menuItems];
    int menuIndex;

    DisplayUtils* displayUtils;
};

} // namespace NuggetsInc

#endif // NFC_OPTIONS_STATE_H
