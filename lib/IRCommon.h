//IRCommon.h
#ifndef IR_COMMON_H
#define IR_COMMON_H

#include <Arduino.h>

namespace NuggetsInc
{

const uint8_t MAX_RAW_DATA_SIZE = 100; 
const uint32_t MAGIC_NUMBER = 0xDEADBEEF;

// Enum to represent buttons
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
    BUTTON_COUNT // Total number
};

struct IRData
{
    uint16_t rawData[MAX_RAW_DATA_SIZE];
    uint8_t rawDataLength;
    bool isValid;
};


    
} // namespace NuggetsInc

#endif // IR_COMMON_H
