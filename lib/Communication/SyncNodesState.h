#ifndef SYNC_NODES_STATE_H
#define SYNC_NODES_STATE_H

#include "State.h"
#include "DisplayUtils.h"
#include <vector>
#include <WiFi.h>
#include <esp_now.h>

namespace NuggetsInc {

class SyncNodesState : public AppState {
public:
    SyncNodesState();
    ~SyncNodesState();

    void onEnter() override;
    void onExit() override;
    void update() override;

private:
    DisplayUtils* displayUtils;
    std::vector<String> macAddresses;
    int currentBroadcastIndex;
    unsigned long lastBroadcastTime;
    unsigned long broadcastStartTime;
    bool broadcastInProgress;
    bool broadcastComplete;
    
    static const unsigned long BROADCAST_INTERVAL = 1000; 
    static const unsigned long BROADCAST_TIMEOUT = 10000; 
    
    void loadMacAddresses();
    void startBroadcast();
    void broadcastToNextNode();
    void sendSyncCommand(const String& targetMac);
    void updateDisplay();

    static SyncNodesState* activeInstance;
};

} // namespace NuggetsInc

#endif // SYNC_NODES_STATE_H
