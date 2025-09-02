#ifndef TAGDATA_H
#define TAGDATA_H

#include <array>
#include <map>
#include <vector>
#include <string>
#include <stdint.h>

namespace NuggetsInc {

class TagData {
public:
    // Default constructor
    TagData();

    // Clone Constructor
    TagData(const TagData& other);

    // Method to validate if the provided data is a supported NTAG type
    bool isValidTag(const std::vector<uint8_t>& rawData);

    // Method to parse and populate NFCTagData based on raw data
    static TagData parseRawData(const std::vector<uint8_t>& rawData);

    int ValidateTagData(TagData NFCTagData);

    void addTextRecord(const std::string &text, const std::string &languageCode);

    uint8_t* CheckForTextRecordWithNITag();

    // Data sections based on NTAG structure
    struct UIDAndManufacturer {
        std::array<uint8_t, 7> UID;
        uint8_t manufacturerCode;
    } uidAndManufacturer;

    struct StaticLock {
        std::array<uint8_t, 2> staticLockBytes;
        std::map<int, bool> lockStatus;
    } staticLock;

    struct UserMemory {
        std::vector<std::array<uint8_t, 4>> pages;
        int totalUserMemoryBytes;
        std::map<int, std::string> pageHex;
        std::map<int, std::string> pageASCII;
        std::map<int, int> pageDecimal;
    } userMemory;

    struct DynamicLockAndConfig {
        std::vector<uint8_t> dynamicLockBytes;
        std::map<int, bool> lockStatus;
    } dynamicLockAndConfig;

    struct PasswordAndCounter {
        bool passwordProtectionEnabled;
        std::array<uint8_t, 4> password;
        uint16_t pack;
        uint32_t counter;
    } passwordAndCounter;

    struct Interpretations {
        std::string UIDHex;
        std::string manufacturerHex;
        std::string passwordHex;
    } interpretations;

    // NDEF Record Structure
    struct Record {
        std::string type;
        std::vector<uint8_t> payload;
    };

    // Array of NDEF Records
    std::vector<Record> records;

    // Raw data storage
    std::vector<uint8_t> rawData;

    int tagType;

private:
    // Helper method to determine NTAG type
    static int determineTagType(const std::vector<uint8_t>& rawData);

    // Method to parse NDEF records from user memory
    void parseRecords();

    // Helper to extract a record from the given position
    size_t extractRecord(size_t pos, Record& record);
};

} // namespace NuggetsInc

#endif // TAGDATA_H
