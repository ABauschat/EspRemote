// NFCLogic.cpp
#include "NFCLogic.h"
#include "Colors.h"
#include <Wire.h>
#include "Config.h" 

namespace NuggetsInc {

NFCLogic::NFCLogic(uint8_t irqPin, uint8_t resetPin)
    : nfc(irqPin, resetPin), authenticated(false) {}

NFCLogic::~NFCLogic() {}

bool NFCLogic::initialize() {
    Wire.begin(I2C_SDA, I2C_SCL); 
    nfc.begin();
    uint32_t versiondata = nfc.getFirmwareVersion();
    if (!versiondata) {
        return false; 
    }

    nfc.SAMConfig();
    return true;
}

bool NFCLogic::isTagPresent() {
    uint8_t uid[7];  
    uint8_t uidLength = 0; 

    //Add A Single Vibrator Buzz:: Future Implementation
    return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
}

const std::vector<uint8_t>& NFCLogic::readRawData()
{
    static std::vector<uint8_t> rawData;
    rawData.clear();

    uint8_t uid[7];
    uint8_t uidLength = 0;

    // Read passive target ID
    if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength))
    {
        // Append error code for failure in reading passive target ID
        rawData.push_back(0x01);  // Error code 0x01: Failed to read passive target ID
        return rawData;
    }

    uint8_t ccByte;
    uint8_t ccBytePage[4];

    // Read page 3 to get CC byte
    if (!nfc.ntag2xx_ReadPage(3, ccBytePage))
    {
        // Append error code for failure in reading page 3
        rawData.push_back(0x02);  // Error code 0x02: Failed to read page 3
        return rawData;
    }

    ccByte = ccBytePage[2];

    int MaxPages = 0;
    switch (ccByte)
    {
    case 0x12:
        MaxPages = 45;
        break;

    case 0x3E:
        MaxPages = 135;
        break;

    case 0x6D:
        MaxPages = 225;
        break;

    default:
        // Append error code for unknown CC byte value
        rawData.push_back(ccBytePage[2]);

        return rawData;
    }

    switch (uidLength)
    {
    case 7:
        uint8_t pageData[4];
        for (uint8_t page = 0; page < MaxPages; page++)
        {
            // Read each page of the tag
            if (!nfc.ntag2xx_ReadPage(page, pageData))
            {
                // Append error code for failure in reading page
                rawData.push_back(0x04);  // Error code 0x04: Failed to read page
                return rawData;
            }
            else
            {
                // Append successfully read data to rawData
                rawData.insert(rawData.end(), pageData, pageData + sizeof(pageData));
            }
        }
        break;
    default:
        // Append error code for invalid UID length
        rawData.push_back(0x05);  // Error code 0x05: Invalid UID length
        return rawData;
    }

    return rawData;
}

bool NFCLogic::writeNTAG2xx(uint8_t* targetUID, uint8_t targetUIDLength, const std::vector<String>& dataLines) {
    std::vector<uint8_t> ndefData;
    for (const auto& line : dataLines) {
        for (size_t i = 0; i < line.length(); i += 3) { // "XX " per byte
            if (i + 2 > line.length()) break;
            String byteStr = line.substring(i, i + 2);
            ndefData.push_back(static_cast<uint8_t>(strtol(byteStr.c_str(), NULL, 16)));
        }
    }

    for (size_t page = 4; page < 39 && (page - 4) * 4 < ndefData.size(); page++) {
        uint8_t pageData[4] = {0};
        for (uint8_t i = 0; i < 4; i++) {
            size_t byteIndex = (page - 4) * 4 + i;
            if (byteIndex < ndefData.size()) {
                pageData[i] = ndefData[byteIndex];
            }
        }

        if (!nfc.ntag2xx_WritePage(page, pageData)) {
            return false;
        }

        uint8_t verifyData[4];
        if (!nfc.ntag2xx_ReadPage(page, verifyData)) {
            return false;
        }

        if (memcmp(pageData, verifyData, 4) != 0) {
            return false;
        }
    }

    return true;
}

} // namespace NuggetsInc