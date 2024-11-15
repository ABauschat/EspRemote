#ifndef SETUP_NFC_DEVICE_STATE_H
#define SETUP_NFC_DEVICE_STATE_H

#include "State.h"
#include "NFCLogic.h"
#include "DisplayUtils.h"
#include "Device.h"

namespace NuggetsInc
{

    class SetupNFCDeviceState : public AppState
    {
    public:
        SetupNFCDeviceState();
        ~SetupNFCDeviceState() override;

        void onEnter() override;
        void onExit() override;
        void update() override;

    private:
        NFCLogic *nfcLogic;
        DisplayUtils *displayUtils;
        void displaySetupInstructions();
    };

} // namespace NuggetsInc

#endif // SETUP_NFC_DEVICE_STATE_H
