#include "TagData.h"
#include <stdexcept>
#include <sstream>
#include <iomanip>

namespace NuggetsInc {

// Default constructor to initialize an empty TagData instance
TagData::TagData() : tagType(0) {}

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

    uint8_t ccByte = rawData[12];  // Page 3, Byte 0

    if (ccByte == 0x12) return 213;  // NTAG213
    if (ccByte == 0x3E) return 215;  // NTAG215
    if (ccByte == 0x6D) return 216;  // NTAG216

    return 0;  // Unknown tag type
}

// Parse and populate TagData based on validated raw data
TagData TagData::parseRawData(const std::vector<uint8_t>& rawData) {
    // Create a new TagData object
    TagData tagData;

    if (!tagData.isValidTag(rawData)) {
        throw std::logic_error("Invalid or unsupported tag type");
    }

    // Set the tagType for struct population
    tagData.tagType = tagData.determineTagType(rawData);

    // Set user memory size and pages based on tag type
    if (tagData.tagType == 213) {
        tagData.userMemory.totalUserMemoryBytes = 144;
        tagData.userMemory.pages.resize(36);
        tagData.dynamicLockAndConfig.dynamicLockBytes.resize(4);  // Pages 40-43
    } else if (tagData.tagType == 215) {
        tagData.userMemory.totalUserMemoryBytes = 540;
        tagData.userMemory.pages.resize(126);
        tagData.dynamicLockAndConfig.dynamicLockBytes.resize(4);  // Pages 130-133
    } else if (tagData.tagType == 216) {
        tagData.userMemory.totalUserMemoryBytes = 888;
        tagData.userMemory.pages.resize(206);
        tagData.dynamicLockAndConfig.dynamicLockBytes.resize(4);  // Pages 210-213
    }

    // Populate UID and Manufacturer Data
    tagData.uidAndManufacturer.manufacturerCode = rawData[0];
    for (int i = 0; i < 7; ++i) {
        tagData.uidAndManufacturer.UID[i] = rawData[i];
    }

    // Convert UID to hexadecimal string
    std::stringstream uidHexStream;
    for (uint8_t byte : tagData.uidAndManufacturer.UID) {
        uidHexStream << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    tagData.interpretations.UIDHex = uidHexStream.str();
    tagData.interpretations.manufacturerHex = "0x04";

    // Populate Static Lock Bytes (Page 3)
    tagData.staticLock.staticLockBytes[0] = rawData[12];
    tagData.staticLock.staticLockBytes[1] = rawData[13];
    for (int i = 0; i < 2; ++i) {
        uint8_t byte = tagData.staticLock.staticLockBytes[i];
        for (int bit = 0; bit < 8; ++bit) {
            tagData.staticLock.lockStatus[i * 8 + bit] = (byte >> bit) & 1;
        }
    }

    // Populate User Memory Pages
    for (size_t i = 4; i < rawData.size() / 4 && i - 4 < tagData.userMemory.pages.size(); ++i) {
        std::copy(rawData.begin() + i * 4, rawData.begin() + (i + 1) * 4, tagData.userMemory.pages[i - 4].begin());
    }

    // Populate Dynamic Lock Bytes based on tag type
    size_t dynamicLockPageStart = 0;
    if (tagData.tagType == 213) {
        dynamicLockPageStart = 40;  // Pages 40-43
    } else if (tagData.tagType == 215) {
        dynamicLockPageStart = 130;  // Pages 130-133
    } else if (tagData.tagType == 216) {
        dynamicLockPageStart = 210;  // Pages 210-213
    }

    for (size_t i = 0; i < tagData.dynamicLockAndConfig.dynamicLockBytes.size(); ++i) {
        tagData.dynamicLockAndConfig.dynamicLockBytes[i] = rawData[dynamicLockPageStart * 4 + i];
        uint8_t byte = tagData.dynamicLockAndConfig.dynamicLockBytes[i];
        for (int bit = 0; bit < 8; ++bit) {
            tagData.dynamicLockAndConfig.lockStatus[dynamicLockPageStart * 8 + bit] = (byte >> bit) & 1;
        }
    }

    // Populate Password and Counter based on tag type
    if (tagData.tagType == 213) {
        for (size_t i = 0; i < 4; ++i) {
            tagData.passwordAndCounter.password[i] = rawData[44 * 4 + i];  // Page 44
        }
        tagData.passwordAndCounter.pack = (rawData[44 * 4 + 4] << 8) | rawData[44 * 4 + 5];
        tagData.passwordAndCounter.counter = (rawData[44 * 4 + 6] << 16) | (rawData[44 * 4 + 7] << 8) | rawData[44 * 4 + 8];

    } else if (tagData.tagType == 215) {
        for (size_t i = 0; i < 4; ++i) {
            tagData.passwordAndCounter.password[i] = rawData[133 * 4 + i];  // Page 133
        }
        tagData.passwordAndCounter.pack = (rawData[134 * 4] << 8) | rawData[134 * 4 + 1];
        tagData.passwordAndCounter.counter = (rawData[136 * 4] << 16) | (rawData[136 * 4 + 1] << 8) | rawData[136 * 4 + 2];

    } else if (tagData.tagType == 216) {
        for (size_t i = 0; i < 4; ++i) {
            tagData.passwordAndCounter.password[i] = rawData[214 * 4 + i];  // Page 214
        }
        tagData.passwordAndCounter.pack = (rawData[215 * 4] << 8) | rawData[215 * 4 + 1];
        tagData.passwordAndCounter.counter = (rawData[216 * 4] << 16) | (rawData[216 * 4 + 1] << 8) | rawData[216 * 4 + 2];
    }

    // Populate Password Protection Enabled Flag
    size_t configPage = (tagData.tagType == 213) ? 42 : (tagData.tagType == 215) ? 131 : 211;
    uint8_t configByte = rawData[configPage * 4 + 3];  // Last byte of the configuration page
    tagData.passwordAndCounter.passwordProtectionEnabled = (configByte & 0x80) != 0;

    // Convert Password to Hex String for Interpretation
    std::stringstream passwordHexStream;
    for (uint8_t byte : tagData.passwordAndCounter.password) {
        passwordHexStream << std::hex << std::setw(2) << std::setfill('0') << (int)byte;
    }
    tagData.interpretations.passwordHex = passwordHexStream.str();

    // Return the populated TagData
    return tagData;
}

} // namespace NuggetsInc
