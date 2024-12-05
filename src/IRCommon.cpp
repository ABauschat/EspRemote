//IRCommon.cpp
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


    String LoadIRData (IRData buttonIRData[]){
        String result = "";

        // Initialize LittleFS
        if (!LittleFS.begin(false))
        {
            result += "Failed to mount LittleFS.";
            return result;
        }

        File file = LittleFS.open("/irData.bin", FILE_READ);

        if (!file)
        {
            result += "No IR data file found.";
            return result;
        }

        uint32_t magicNumber;

        if (file.read(reinterpret_cast<uint8_t *>(&magicNumber), sizeof(magicNumber)) != sizeof(magicNumber))
        {
            result += "Failed to read Magic Number.";
            file.close();
            return result;
        }

        if (magicNumber != MAGIC_NUMBER)
        {
            result += "Invalid Magic Number. Data may be corrupted.";
            file.close();
            return result;
        }

        size_t bytesRead = file.read(reinterpret_cast<uint8_t *>(buttonIRData), sizeof(buttonIRData));

        if (bytesRead != sizeof(buttonIRData))
        {
            result += "Failed to read all IR data from flash.";
            file.close();
            return result;
        }

        // Mark all buttons as valid

        for (int i = 0; i < BUTTON_COUNT; i++)
        {
            buttonIRData[i].isValid = true;
        }

        result += "IR data successfully loaded from flash.";

        file.close();
        return result;
    }

    String SendIRData(IRData buttonIRData[], ButtonType button)
    {
        if (!buttonIRData[button].isValid)
        {
            return "No IR data stored for this button.";
        }

        IrSender.sendRaw(buttonIRData[button].rawData, buttonIRData[button].rawDataLength, 38);

        return "IR data sent.";
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
            // Using the MICROS_PER_TICK defined in IRremote.hpp
            buttonIRData.rawData[i - 1] = IrReceiver.decodedIRData.rawDataPtr->rawbuf[i] * MICROS_PER_TICK;
        }

        buttonIRData.rawDataLength = len - 1;
        buttonIRData.isValid = true;

        return buttonIRData;
    }


} // namespace NuggetsInc