#include "TagData.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>
#include <algorithm>

namespace NuggetsInc {

// Default constructor to initialize an empty TagData instance
TagData::TagData() : tagType(0) {}

// Clone Constructor
TagData::TagData(const TagData& other) {
    uidAndManufacturer = other.uidAndManufacturer;
    staticLock = other.staticLock;
    userMemory = other.userMemory;
    dynamicLockAndConfig = other.dynamicLockAndConfig;
    passwordAndCounter = other.passwordAndCounter;
    interpretations = other.interpretations;
    rawData = other.rawData;
    tagType = other.tagType;
    records = other.records;
}

// Method to validate if the provided data is a supported NTAG type
bool TagData::isValidTag(const std::vector<uint8_t>& rawData) {
    if (rawData.size() < 16) return false;

    // Verify manufacturer code directly in rawData
    if (rawData[0] != 0x04) return false;

    // Determine tag type based on Capability Container byte
    return determineTagType(rawData) != 0;
}

// Determine NTAG type based on CC byte in rawData
int TagData::determineTagType(const std::vector<uint8_t>& rawData) {
    if (rawData.size() < 13) return 0;

    uint8_t ccByte = rawData[14];  // Page 3, Byte 0

    if (ccByte == 0x12) return 213;  // NTAG213
    if (ccByte == 0x3E) return 215;  // NTAG215
    if (ccByte == 0x6D) return 216;  // NTAG216

    return 0;  // Unknown tag type
}

// Parse and populate NFCTagData based on validated raw data
TagData TagData::parseRawData(const std::vector<uint8_t>& rawData) {
    // Create a new NFCTagData object
    TagData NFCTagData;

    if (!NFCTagData.isValidTag(rawData)) {
        return NFCTagData;
    }

    // Set the tagType for struct population
    NFCTagData.tagType = NFCTagData.determineTagType(rawData);

    // Set user memory size and pages based on tag type
    if (NFCTagData.tagType == 213) {
        NFCTagData.userMemory.totalUserMemoryBytes = 144;
        NFCTagData.userMemory.pages.resize(36);
        NFCTagData.dynamicLockAndConfig.dynamicLockBytes.resize(4);  // Pages 40-43
    } else if (NFCTagData.tagType == 215) {
        NFCTagData.userMemory.totalUserMemoryBytes = 540;
        NFCTagData.userMemory.pages.resize(126);
        NFCTagData.dynamicLockAndConfig.dynamicLockBytes.resize(4);  // Pages 130-133
    } else if (NFCTagData.tagType == 216) {
        NFCTagData.userMemory.totalUserMemoryBytes = 888;
        NFCTagData.userMemory.pages.resize(206);
        NFCTagData.dynamicLockAndConfig.dynamicLockBytes.resize(4);  // Pages 210-213
    }

    // Populate UID and Manufacturer Data
    NFCTagData.uidAndManufacturer.manufacturerCode = rawData[0];
    for (int i = 0; i < 7; ++i) {
        NFCTagData.uidAndManufacturer.UID[i] = rawData[i];
    }

    // Convert UID to hexadecimal string
    std::stringstream uidHexStream;
    for (uint8_t byte : NFCTagData.uidAndManufacturer.UID) {
        uidHexStream << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    NFCTagData.interpretations.UIDHex = uidHexStream.str();
    NFCTagData.interpretations.manufacturerHex = "0x04";

    // Populate Static Lock Bytes (Page 3)
    NFCTagData.staticLock.staticLockBytes[0] = rawData[12];
    NFCTagData.staticLock.staticLockBytes[1] = rawData[13];
    for (int i = 0; i < 2; ++i) {
        uint8_t byte = NFCTagData.staticLock.staticLockBytes[i];
        for (int bit = 0; bit < 8; ++bit) {
            NFCTagData.staticLock.lockStatus[i * 8 + bit] = (byte >> bit) & 1;
        }
    }

    // Populate User Memory Pages
    for (size_t i = 4; i < rawData.size() / 4 && i - 4 < NFCTagData.userMemory.pages.size(); ++i) {
        std::copy(rawData.begin() + i * 4, rawData.begin() + (i + 1) * 4, NFCTagData.userMemory.pages[i - 4].begin());
    }

    // Set Raw Data
    NFCTagData.rawData = rawData;

    // Populate Dynamic Lock Bytes based on tag type
    size_t dynamicLockPageStart = 0;
    if (NFCTagData.tagType == 213) {
        dynamicLockPageStart = 40;  // Pages 40-43
    } else if (NFCTagData.tagType == 215) {
        dynamicLockPageStart = 130;  // Pages 130-133
    } else if (NFCTagData.tagType == 216) {
        dynamicLockPageStart = 210;  // Pages 210-213
    }

    for (size_t i = 0; i < NFCTagData.dynamicLockAndConfig.dynamicLockBytes.size(); ++i) {
        NFCTagData.dynamicLockAndConfig.dynamicLockBytes[i] = rawData[dynamicLockPageStart * 4 + i];
        uint8_t byte = NFCTagData.dynamicLockAndConfig.dynamicLockBytes[i];
        for (int bit = 0; bit < 8; ++bit) {
            NFCTagData.dynamicLockAndConfig.lockStatus[dynamicLockPageStart * 8 + bit] = (byte >> bit) & 1;
        }
    }

    // Populate Password and Counter based on tag type
    if (NFCTagData.tagType == 213) {
        for (size_t i = 0; i < 4; ++i) {
            NFCTagData.passwordAndCounter.password[i] = rawData[44 * 4 + i];  // Page 44
        }
        NFCTagData.passwordAndCounter.pack = (rawData[44 * 4 + 4] << 8) | rawData[44 * 4 + 5];
        NFCTagData.passwordAndCounter.counter = (rawData[44 * 4 + 6] << 16) | (rawData[44 * 4 + 7] << 8) | rawData[44 * 4 + 8];

    } else if (NFCTagData.tagType == 215) {
        for (size_t i = 0; i < 4; ++i) {
            NFCTagData.passwordAndCounter.password[i] = rawData[133 * 4 + i];  // Page 133
        }
        NFCTagData.passwordAndCounter.pack = (rawData[134 * 4] << 8) | rawData[134 * 4 + 1];
        NFCTagData.passwordAndCounter.counter = (rawData[136 * 4] << 16) | (rawData[136 * 4 + 1] << 8) | rawData[136 * 4 + 2];

    } else if (NFCTagData.tagType == 216) {
        for (size_t i = 0; i < 4; ++i) {
            NFCTagData.passwordAndCounter.password[i] = rawData[214 * 4 + i];  // Page 214
        }
        NFCTagData.passwordAndCounter.pack = (rawData[215 * 4] << 8) | rawData[215 * 4 + 1];
        NFCTagData.passwordAndCounter.counter = (rawData[216 * 4] << 16) | (rawData[216 * 4 + 1] << 8) | rawData[216 * 4 + 2];
    }

    // Populate Password Protection Enabled Flag
    size_t configPage = (NFCTagData.tagType == 213) ? 42 : (NFCTagData.tagType == 215) ? 131 : 211;
    uint8_t configByte = rawData[configPage * 4 + 3];  // Last byte of the configuration page
    NFCTagData.passwordAndCounter.passwordProtectionEnabled = (configByte & 0x80) != 0;

    // Convert Password to Hex String for Interpretation
    std::stringstream passwordHexStream;
    for (uint8_t byte : NFCTagData.passwordAndCounter.password) {
        passwordHexStream << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    NFCTagData.interpretations.passwordHex = passwordHexStream.str();

    // Parse NDEF Records
    NFCTagData.parseRecords();

    // Return the populated NFCTagData
    return NFCTagData;
}

int TagData::ValidateTagData(TagData NFCTagData) {
    // Ensure the UID and Manufacturer are correctly populated
    if (NFCTagData.uidAndManufacturer.UID.empty()) {
        return 1;  // Invalid UID or Manufacturer Code
    }

    // Check for valid tagType
    if (NFCTagData.tagType != 213 && NFCTagData.tagType != 215 && NFCTagData.tagType != 216) {
        return 2;  // Invalid tag type
    }

    // Validate Static Lock - Ensure lock status are consistent with the bytes
    for (auto& entry : NFCTagData.staticLock.lockStatus) {
        if (entry.second != 0 && entry.second != 1) {
            return 3;  // Invalid lock status
        }
    }

    // Validate User Memory - Ensure the pages are not empty
    if (NFCTagData.userMemory.pages.empty()) {
        return 4;  // User memory pages are missing
    }

    // Validate Dynamic Lock - Ensure the lock status is consistent with the dynamic lock bytes
    for (auto& entry : NFCTagData.dynamicLockAndConfig.lockStatus) {
        if (entry.second != 0 && entry.second != 1) {
            return 5;  // Invalid lock status
        }
    }

    // All checks passed, the tag data is valid
    return 0;
}

void TagData::parseRecords() {
    // Starting position after initial pages (skip first 4 pages, which are 16 bytes)
    size_t pos = 4 * 4;

    while (pos < rawData.size()) {
        // Check if the current byte is the start byte 0x01
        if (rawData[pos] == 0x01) {
            Record record;
            size_t recordLength = extractRecord(pos, record);
            if (recordLength == 0) {
                // Failed to extract record, move to next byte
                pos++;
                continue;
            }
            records.push_back(record);
            pos += recordLength;
        } else {
            // Move to the next byte
            pos++;
        }
    }
}

// Helper to extract a record from the given position
size_t TagData::extractRecord(size_t pos, Record& record) {
    size_t startPos = pos;

    // Check for the start byte 0x01
    if (rawData[pos] != 0x01) {
        return 0;
    }

    if (pos + 2 >= rawData.size()) return 0;

    // Read the type from rawData[pos + 1]
    uint8_t typeByte = rawData[pos + 2];
    record.type = static_cast<char>(typeByte);

    // Look for the end byte 0xFE or next start byte 0x01
    size_t payloadStart = pos + 2;
    size_t payloadEnd = pos + 2;
    bool endFound = false;
    bool nextStartFound = false;

    while (payloadEnd < rawData.size()) {
        if (rawData[payloadEnd] == 0xFE) {
            endFound = true;
            break;
        }
        if (rawData[payloadEnd] == 0x01) {
            nextStartFound = true;
            break;
        }
        payloadEnd++;
    }

    if (!endFound && !nextStartFound) {
        // End byte not found
        return 0;
    }

    // Extract the raw payload including placeholders
    std::vector<uint8_t> rawPayload(rawData.begin() + payloadStart, rawData.begin() + payloadEnd);

    // Remove placeholders: first byte and last byte
    if (rawPayload.size() < 2) return 0; // Not enough data after removing placeholders
    rawPayload.erase(rawPayload.begin(), rawPayload.begin()+2); // Remove the 0x01 and first placeholder byte
    
    if (nextStartFound) { // If next start byte found, remove the last placeholder byte
        rawPayload.pop_back(); // Remove last placeholder byte
    }
    // If the type is 'T', remove the language identifier (first 2 bytes)
    if (record.type == "T") {
        if (rawPayload.size() < 2) return 0; // Not enough data to remove language identifier
        rawPayload.erase(rawPayload.begin(), rawPayload.begin() + 2); // Remove language identifier
    }

    // Set the cleaned payload
    record.payload = rawPayload;

    // Return the length consumed
    return payloadEnd - startPos;
}

} // namespace NuggetsInc
