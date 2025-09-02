// NFCLogic.h
#ifndef NFCLOGIC_H
#define NFCLOGIC_H

#include <Adafruit_PN532.h>
#include <Arduino.h>
#include <vector>
#include <String.h>
#include "TagData.h"

namespace NuggetsInc {
class NFCLogic {
public:
    NFCLogic(uint8_t irqPin, uint8_t resetPin);
    ~NFCLogic();

    bool initialize();

    bool isTagPresent();
    const std::vector<uint8_t>& readRawData();

    bool writeTagData(const TagData& tagData);
    bool overwriteRecords(uint16_t tagType);

private:
    Adafruit_PN532 nfc;
    bool authenticated;
};

} // namespace NuggetsInc

#endif // NFCLOGIC_H
