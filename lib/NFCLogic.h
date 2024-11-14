// NFCLogic.h
#ifndef NFCLOGIC_H
#define NFCLOGIC_H

#include <Adafruit_PN532.h>
#include <Arduino.h>
#include <vector>
#include <String.h>

namespace NuggetsInc {
class NFCLogic {
public:
    NFCLogic(uint8_t irqPin, uint8_t resetPin);
    ~NFCLogic();

    bool initialize();

    bool isTagPresent();
    const std::vector<uint8_t>& readRawData();

private:
    Adafruit_PN532 nfc;
    bool authenticated;

    bool writeNTAG2xx(uint8_t* targetUID, uint8_t targetUIDLength, const std::vector<String>& dataLines);
};

} // namespace NuggetsInc

#endif // NFCLOGIC_H
