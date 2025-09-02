// MessageTypes.h
#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <stdint.h>

#pragma pack(push, 1)
struct struct_message
{
    uint32_t messageID;
    uint8_t SenderMac[6];
    char messageType[10];
    uint8_t commandID;
    char data[50];
    uint8_t destinationMac[6]; // Target device MAC for routed delivery; zeros for direct/legacy
    char path[128];            // Comma-separated MAC path taken so far; empty for direct
};

enum DisplayCommandID : uint8_t {
    CMD_CLEAR_DISPLAY         = 0x01,
    CMD_DISPLAY_MESSAGE       = 0x02,
    CMD_NEW_TERMINAL_DISPLAY  = 0x03,
    CMD_ADD_TO_TERMINAL       = 0x04,
    CMD_PRINTLN               = 0x05,
    CMD_PRINT                 = 0x06,
    CMD_SET_CURSOR            = 0x07,
    CMD_SET_TEXT_SIZE         = 0x08,
    CMD_SET_TEXT_COLOR        = 0x09,
    CMD_FILL_SCREEN           = 0x0A,
    CMD_DRAW_RECT             = 0x0B,
    CMD_FILL_RECT             = 0x0C,
    CMD_BOOOP                 = 0x0D,
    CMD_MOVE_UP               = 0x0E,
    CMD_MOVE_DOWN             = 0x0F,
    CMD_MOVE_LEFT             = 0x10,
    CMD_MOVE_RIGHT            = 0x11,
    CMD_SELECT                = 0x12,
    CMD_BACK                  = 0x13,
    CMD_BEGIN_PLOT            = 0x14,
    CMD_PLOT_POINT            = 0x15,
    CMD_RELAY_CONNECTION      = 0x16,
    CMD_SYNC_NODES            = 0x17,
};
#pragma pack(pop)

#endif // MESSAGE_TYPES_H
