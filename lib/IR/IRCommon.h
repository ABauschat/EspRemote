// IRCommon.h

#ifndef IR_COMMON_H
#define IR_COMMON_H

#include <Arduino.h>
#include "EventManager.h"

namespace NuggetsInc
{

    const uint8_t MAX_RAW_DATA_SIZE = 100;
    const uint32_t MAGIC_NUMBER = 0xDEADBEEF;
    const uint8_t MAX_REMOTE_SLOTS = 10;

    enum ButtonType
    {
        BUTTON_ACTION_ONE,
        BUTTON_ACTION_TWO,
        BUTTON_UP,
        BUTTON_DOWN,
        BUTTON_LEFT,
        BUTTON_RIGHT,
        BUTTON_SELECT,
        BUTTON_BACK,
        BUTTON_COUNT
    };

    struct IRData
    {
        uint16_t rawData[MAX_RAW_DATA_SIZE];
        uint8_t rawDataLength;
        bool isValid;
    };

    struct RemoteData
    {
        IRData buttonIRData[BUTTON_COUNT];
    };

    // Structure to hold send requests
    struct SendRequest {
        ButtonType button;
        uint8_t slot;
    };

    void BeginIrSender();
    String LoadIRData(RemoteData remotes[]);
    String SendIRData(RemoteData remotes[], ButtonType button, uint8_t slot);
    ButtonType mapEventTypeToButtonType(EventType eventType);

    void BeginIrReceiver();
    void StopIrReceiver();
    void RecieverResume();
    bool RecieverIsIdle();
    bool RecieverDecode();
    IRData DecodeIRData();

    // Task-related functions
    void InitializeSendTask(RemoteData remotes[]);
    bool EnqueueSendRequest(ButtonType button, uint8_t slot);

} // namespace NuggetsInc

#endif // IR_COMMON_H
