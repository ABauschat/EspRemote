#ifndef CLONE_NFC_STATE_H
#define CLONE_NFC_STATE_H

#include "State.h"
#include <vector>
#include <String.h>
#include "NFCLogic.h"
#include "DisplayUtils.h"
#include "Colors.h"
#include "TagData.h"
#include "Tab.h"

namespace NuggetsInc {

class CloneNFCState : public AppState {
public:
    CloneNFCState();
    ~CloneNFCState();

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    // NFC functions
    void readNFCTag();

    bool cloneTag();
    bool reformatChip();
    void handleScroll(EventType eventType);

    bool tagDetected;
    bool cloneTagData;          // (This flag may be repurposed or removed)
    bool displayNeedsRefresh;
    Tab currentTabWindow;
    NFCLogic* nfcLogic;
    DisplayUtils* displayUtils;
    TagData* currentTagData;
    void displayTagInformation();
    std::vector<Tab> tabs;
    int currentTabIndex;
    void populateTabs();
};

} // namespace NuggetsInc

#endif // CLONE_NFC_STATE_H
