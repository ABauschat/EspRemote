// NFCLogic.h
#ifndef NFCLOGIC_H
#define NFCLOGIC_H

#include <Adafruit_PN532.h>
#include <Arduino.h>
#include <vector>
#include <String.h>

namespace NuggetsInc {

// Define constants for tag types
#define TAG_TYPE_MIFARE_ULTRALIGHT "MIFARE Ultralight"
#define TAG_TYPE_MIFARE_CLASSIC "MIFARE Classic"
#define TAG_TYPE_NTAG2XX "NTAG2xx"
#define TAG_TYPE_UNKNOWN "Unknown"

// Define NDEF record structure
struct NDEFRecord {
    String type;
    String payload;
};

// Define a struct to hold tag data
struct TagData {
    String tagType;
    String data;
    std::vector<NDEFRecord> ndefRecords;
    String availableSpace;
    uint8_t uid[7];
    uint8_t uidLength;
};

class NFCLogic {
public:
    NFCLogic(uint8_t irqPin, uint8_t resetPin);
    ~NFCLogic();

    bool initialize();
    bool readTag(uint8_t* uid, uint8_t* uidLength);
    String getTagType(uint8_t uidLength);
    String readTagData(const String& tagType, uint8_t* uid, uint8_t uidLength);
    bool cloneTagData(const String& clonedTagType, uint8_t* targetUID, uint8_t targetUIDLength, const std::vector<String>& dataLines);

    // New methods
    bool readAndParseTagData(TagData& tagData);
    bool validateAndCloneTag(const String& clonedTagType, std::vector<String>& dataLines);

    std::vector<NDEFRecord> parseNDEF(const uint8_t* data, size_t length);

private:
    Adafruit_PN532 nfc;
    bool authenticated;

    // NFC functions
    String readMIFAREClassic(uint8_t* uid, uint8_t uidLen);
    String readNTAG2xx(uint8_t* uid, uint8_t uidLen);
    bool writeMIFAREClassic(uint8_t* targetUID, uint8_t targetUIDLength, const std::vector<String>& dataLines);
    bool writeNTAG2xx(uint8_t* targetUID, uint8_t targetUIDLength, const std::vector<String>& dataLines);
    bool writeAndVerifyMIFAREClassicBlock(uint8_t blockAddr, const uint8_t* blockData);

    // NDEF Parsing
    String parseURI(const uint8_t* payload, size_t length);
    String parseText(const uint8_t* payload, size_t length);
};

} // namespace NuggetsInc

#endif // NFCLOGIC_H
