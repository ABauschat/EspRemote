// NFCLogic.cpp
#include "NFCLogic.h"
#include "Colors.h"
#include <Wire.h>
#include "Config.h"
#include "Haptics.h"

namespace NuggetsInc
{

    NFCLogic::NFCLogic(uint8_t irqPin, uint8_t resetPin)
        : nfc(irqPin, resetPin), authenticated(false) {}

    NFCLogic::~NFCLogic() {}

    bool NFCLogic::initialize()
    {
        Wire.begin(I2C_SDA, I2C_SCL);
        nfc.begin();
        uint32_t versiondata = nfc.getFirmwareVersion();
        if (!versiondata)
        {
            return false;
        }

        nfc.SAMConfig();
        return true;
    }

    bool NFCLogic::isTagPresent()
    {
        uint8_t uid[7];
        uint8_t uidLength = 0;

        return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 100);
    }

    const std::vector<uint8_t> &NFCLogic::readRawData()
    {
       Haptics::getInstance().singleVibration();

        static std::vector<uint8_t> rawData;
        rawData.clear();

        uint8_t uid[7];
        uint8_t uidLength = 0;

        // Read passive target ID
        if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength))
        {
            // Append error code for failure in reading passive target ID
            rawData.push_back(0x01); // Error code 0x01: Failed to read passive target ID
            return rawData;
        }

        uint8_t ccByte;
        uint8_t ccBytePage[4];

        // Read page 3 to get CC byte
        if (!nfc.ntag2xx_ReadPage(3, ccBytePage))
        {
            // Append error code for failure in reading page 3
            rawData.push_back(0x02); // Error code 0x02: Failed to read page 3
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
                    rawData.push_back(0x04); // Error code 0x04: Failed to read page
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
            rawData.push_back(0x05); // Error code 0x05: Invalid UID length
            return rawData;
        }

         Haptics::getInstance().doubleVibration();

        return rawData;
    }

    bool NFCLogic::writeTagData(const TagData &tagData)
    {
        // Verify that a tag is present before writing
        if (!isTagPresent())
        {
            // Optionally, display a message or handle the absence of a tag
            return false;
        }

        Haptics::getInstance().singleVibration();

        // Start writing from page 4 (user memory area)
        size_t startPage = 4;

        // Calculate the number of pages to write
        // Each page is 4 bytes
        size_t totalBytes = tagData.rawData.size();

        // Ensure we have enough data
        if (totalBytes <= 16)
        {
            return false; // Not enough data to write user memory
        }

        size_t userDataBytes = totalBytes - 16;      // Subtract the first 16 bytes (pages 0-3)
        size_t totalPages = (userDataBytes + 3) / 4; // Round up to full pages

        // Determine max pages based on tag type
        size_t maxPages = 0;
        if (tagData.tagType == 213)
        {
            maxPages = 36; // NTAG213 user memory pages
        }
        else if (tagData.tagType == 215)
        {
            maxPages = 126; // NTAG215 user memory pages
        }
        else if (tagData.tagType == 216)
        {
            maxPages = 222; // NTAG216 user memory pages
        }
        else
        {
            return false; // Unknown tag type
        }

        // Adjust totalPages if it exceeds maxPages
        if (totalPages > maxPages)
        {
            totalPages = maxPages;
        }

        for (size_t pageIndex = 0; pageIndex < totalPages; ++pageIndex)
        {
            size_t page = startPage + pageIndex;
            size_t byteIndex = 16 + pageIndex * 4; // Starting from rawData[16]

            uint8_t pageData[4] = {0};

            // Copy up to 4 bytes from rawData to pageData
            for (size_t i = 0; i < 4; ++i)
            {
                if (byteIndex + i < totalBytes)
                {
                    pageData[i] = tagData.rawData[byteIndex + i];
                }
                else
                {
                    pageData[i] = 0; // Pad with zeros if necessary
                }
            }

            // Write the page data to the tag
            if (!nfc.ntag2xx_WritePage(page, pageData))
            {
                return false; // Failed to write page
            }

            uint8_t verifyData[4];
            if (!nfc.ntag2xx_ReadPage(page, verifyData))
            {
                return false; // Failed to read page for verification
            }

            if (memcmp(pageData, verifyData, 4) != 0)
            {
                return false; // Verification failed
            }
        }

        Haptics::getInstance().doubleVibration();

        return true; // Successfully written all pages
    }

    bool NFCLogic::overwriteRecords(uint16_t tagType)
    {
        // Verify that a tag is present before overwriting.
        if (!isTagPresent())
        {
            return false;
        }
    
        // Provide initial haptic feedback.
        Haptics::getInstance().singleVibration();
    
        // Determine the number of record pages (user data) that can safely be overwritten.
        // This ensures that reserved security/OTP pages at the end remain intact.
        size_t recordPages = 0;
        if (tagType == 213)
        {
            // For NTAG213: Total pages = 45, but pages 0-3 are reserved,
            // so pages 4–39 (36 pages) contain NDEF data.
            recordPages = 36;
        }
        else if (tagType == 215)
        {
            // For NTAG215: Total pages = 135, user memory pages 4–129 (126 pages).
            recordPages = 126;
        }
        else if (tagType == 216)
        {
            // For NTAG216: Although the chip reports 225 pages total,
            // only pages 4–(4+216-1) (i.e. pages 4–219) are considered safe for user data.
            recordPages = 216;  // Adjust as needed if your chip uses a different layout.
        }
        else
        {
            return false; // Unknown tag type.
        }
    
        // Write a valid empty NDEF TLV to page 4.
        // This blank NDEF TLV tells readers that the tag is formatted but contains no NDEF message.
        uint8_t blankNDEF[4] = {0x03, 0x00, 0xFE, 0x00};
        if (!nfc.ntag2xx_WritePage(4, blankNDEF))
        {
            return false; // Failed to write page 4.
        }
        {
            uint8_t verifyData[4];
            if (!nfc.ntag2xx_ReadPage(4, verifyData) ||
                memcmp(blankNDEF, verifyData, 4) != 0)
            {
                return false; // Verification failed for page 4.
            }
        }
    
        // Overwrite remaining record pages with zeros, starting from page 5.
        // (We leave page 4 intact so that the tag retains a valid TLV header.)
        for (size_t page = 5; page < 4 + recordPages; ++page)
        {
            uint8_t zeroData[4] = {0, 0, 0, 0};
    
            // Write the page filled with zeros.
            if (!nfc.ntag2xx_WritePage(page, zeroData))
            {
                return false; // Failed to write page.
            }
    
            // Verify the written data.
            uint8_t verifyData[4];
            if (!nfc.ntag2xx_ReadPage(page, verifyData) ||
                memcmp(zeroData, verifyData, 4) != 0)
            {
                return false; // Verification failed.
            }
        }
    
        // Provide final haptic feedback.
        Haptics::getInstance().doubleVibration();
    
        return true; // Successfully cleared the record area while preserving tag formatting.
    }
    

} // namespace NuggetsInc