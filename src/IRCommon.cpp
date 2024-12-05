// IRCommon.cpp

#include "IRCommon.h"
#include <LittleFS.h>
#include <IRremote.hpp>

#define IR_RECEIVE_PIN 14
#define IR_SEND_PIN 15

namespace NuggetsInc
{

    ButtonType mapEventTypeToButtonType(EventType eventType)
    {
        switch (eventType)
        {
        case EVENT_ACTION_ONE:
            return BUTTON_ACTION_ONE;
        case EVENT_ACTION_TWO:
            return BUTTON_ACTION_TWO;
        case EVENT_UP:
            return BUTTON_UP;
        case EVENT_DOWN:
            return BUTTON_DOWN;
        case EVENT_LEFT:
            return BUTTON_LEFT;
        case EVENT_RIGHT:
            return BUTTON_RIGHT;
        case EVENT_SELECT:
            return BUTTON_SELECT;
        case EVENT_BACK:
            return BUTTON_BACK;
        default:
            return BUTTON_COUNT;
        }
    }

    void BeginIrSender()
    {
        IrSender.begin(IR_SEND_PIN);
    }

    String LoadIRData(RemoteData remotes[])
    {
        String result = "";

        if (!LittleFS.begin(false))
        {
            result += "Failed to mount LittleFS.";
            return result;
        }

        for (uint8_t slot = 0; slot < MAX_REMOTE_SLOTS; slot++)
        {
            String filename = "/irData" + String(slot) + ".bin";
            File file = LittleFS.open(filename, FILE_READ);

            if (!file)
            {
                result += "No IR data file found for slot " + String(slot) + ".\n";
                continue;
            }

            uint32_t magicNumber;

            if (file.read(reinterpret_cast<uint8_t *>(&magicNumber), sizeof(magicNumber)) != sizeof(magicNumber))
            {
                result += "Failed to read Magic Number for slot " + String(slot) + ".\n";
                file.close();
                continue;
            }

            if (magicNumber != MAGIC_NUMBER)
            {
                result += "Invalid Magic Number for slot " + String(slot) + ". Data may be corrupted.\n";
                file.close();
                continue;
            }

            size_t bytesRead = file.read(reinterpret_cast<uint8_t *>(remotes[slot].buttonIRData), sizeof(remotes[slot].buttonIRData));

            if (bytesRead != sizeof(remotes[slot].buttonIRData))
            {
                result += "Failed to read all IR data for slot " + String(slot) + ".\n";
                file.close();
                continue;
            }

            for (int i = 0; i < BUTTON_COUNT; i++)
            {
                remotes[slot].buttonIRData[i].isValid = true;
            }

            result += "IR data successfully loaded for slot " + String(slot) + ".\n";
            file.close();
        }

        return result;
    }

    String SendIRData(RemoteData remotes[], ButtonType button, uint8_t slot)
    {
        if (slot >= MAX_REMOTE_SLOTS)
        {
            return "Invalid slot number.";
        }

        if (!remotes[slot].buttonIRData[button].isValid)
        {
            return "No IR data stored for this button in slot " + String(slot) + ".";
        }

        IrSender.sendRaw(remotes[slot].buttonIRData[button].rawData, remotes[slot].buttonIRData[button].rawDataLength, 38);

        return "IR data sent from slot " + String(slot) + ".";
    }

    void BeginIrReceiver()
    {
        IrReceiver.begin(IR_RECEIVE_PIN);
    }

    void StopIrReceiver()
    {
        IrReceiver.stop();
    }

    void RecieverResume()
    {
        IrReceiver.resume();
    }

    bool RecieverIsIdle()
    {
        return IrReceiver.isIdle();
    }

    bool RecieverDecode()
    {
        return IrReceiver.decode();
    }

    IRData DecodeIRData()
    {
        IRData buttonIRData;

        uint8_t len = IrReceiver.decodedIRData.rawDataPtr->rawlen;

        if (len - 1 > MAX_RAW_DATA_SIZE)
        {
            len = MAX_RAW_DATA_SIZE + 1;
        }

        for (uint8_t i = 1; i < len; i++)
        {
            buttonIRData.rawData[i - 1] = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
        }

        buttonIRData.rawDataLength = len - 1;
        buttonIRData.isValid = true;

        return buttonIRData;
    }

} // namespace NuggetsInc
