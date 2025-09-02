#ifndef MAC_ADDRESS_MENU_STATE_H
#define MAC_ADDRESS_MENU_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <vector>

namespace NuggetsInc {

class MacAddressMenuState : public AppState {
public:
    MacAddressMenuState();
    ~MacAddressMenuState();

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    void displayMenu();
    void loadMacAddresses();
    void scrollUp();
    void scrollDown();
    
    DisplayUtils* displayUtils;
    std::vector<String> macAddresses;
    int selectedIndex;
    int scrollOffset;
    static const int MAX_VISIBLE_ITEMS = 8; // Number of MAC addresses visible on screen
};

} // namespace NuggetsInc

#endif // MAC_ADDRESS_MENU_STATE_H
