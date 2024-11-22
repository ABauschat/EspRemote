// MessageTypes.h
#ifndef MESSAGE_TYPES_H
#define MESSAGE_TYPES_H

#include <stdint.h>

#pragma pack(push, 1)
struct struct_message
{
    uint32_t messageID;
    char messageType[10];
    char command[20];
    char data[50];
};
#pragma pack(pop)

#endif // MESSAGE_TYPES_H
