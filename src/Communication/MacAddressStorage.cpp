#include "MacAddressStorage.h"
#include <LittleFS.h>

namespace NuggetsInc {

const char* MacAddressStorage::MAC_STORAGE_FILE = "/macAddresses.bin";
const uint32_t MacAddressStorage::MAC_MAGIC_NUMBER = 0xDEADBEEF;

MacAddressStorage& MacAddressStorage::getInstance() {
    static MacAddressStorage instance;
    return instance;
}

MacAddressStorage::MacAddressStorage() : initialized(false) {
    macAddresses.reserve(MAX_MAC_ADDRESSES);
}

MacAddressStorage::~MacAddressStorage() {
}

bool MacAddressStorage::init() {
    if (initialized) {
        Serial.println("MacAddressStorage already initialized");
        return true;
    }

    // Try to mount LittleFS if not already mounted
    if (!LittleFS.begin(false)) {
        Serial.println("LittleFS not mounted, trying to mount with format...");
        if (!LittleFS.begin(true)) {
            Serial.println("Failed to mount LittleFS for MAC storage even with format");
            return false;
        }
        Serial.println("LittleFS mounted successfully with format");
    } else {
        Serial.println("LittleFS already mounted or mounted successfully");
    }

    // Test LittleFS functionality
    Serial.printf("LittleFS total bytes: %d\n", LittleFS.totalBytes());
    Serial.printf("LittleFS used bytes: %d\n", LittleFS.usedBytes());

    // Test write capability
    File testFile = LittleFS.open("/test.txt", FILE_WRITE);
    if (testFile) {
        testFile.println("MAC storage test");
        testFile.close();
        Serial.println("LittleFS write test successful");
        LittleFS.remove("/test.txt"); // Clean up
    } else {
        Serial.println("LittleFS write test failed");
        return false;
    }

    bool result = loadMacAddresses();
    if (result) {
        initialized = true;
        Serial.println("MacAddressStorage initialized successfully");
    }
    return result;
}

bool MacAddressStorage::saveMacAddress(const String& macAddress) {
    if (!isValidMacAddress(macAddress)) {
        Serial.println("Invalid MAC address format");
        return false;
    }
    
    // Check if MAC address already exists
    for (const String& existingMac : macAddresses) {
        if (existingMac.equals(macAddress)) {
            Serial.println("MAC address already exists");
            return true; // Not an error, just already exists
        }
    }
    
    // Check if we have space for more MAC addresses
    if (macAddresses.size() >= MAX_MAC_ADDRESSES) {
        Serial.println("Maximum MAC addresses reached");
        return false;
    }
    
    // Add the new MAC address
    macAddresses.push_back(macAddress);
    
    // Save to file
    return saveMacAddressesToFile();
}

std::vector<String> MacAddressStorage::getAllMacAddresses() {
    return macAddresses;
}

bool MacAddressStorage::clearAllMacAddresses() {
    macAddresses.clear();
    
    // Delete the file
    if (LittleFS.exists(MAC_STORAGE_FILE)) {
        return LittleFS.remove(MAC_STORAGE_FILE);
    }
    
    return true;
}

int MacAddressStorage::getMacAddressCount() {
    return macAddresses.size();
}

bool MacAddressStorage::loadMacAddresses() {
    macAddresses.clear();
    
    if (!LittleFS.exists(MAC_STORAGE_FILE)) {
        Serial.println("MAC storage file does not exist, starting fresh");
        return true; // Not an error, just no saved data
    }
    
    File file = LittleFS.open(MAC_STORAGE_FILE, FILE_READ);
    if (!file) {
        Serial.println("Failed to open MAC storage file for reading");
        return false;
    }
    
    // Read and verify magic number
    uint32_t magicNumber;
    if (file.read(reinterpret_cast<uint8_t*>(&magicNumber), sizeof(magicNumber)) != sizeof(magicNumber)) {
        Serial.println("Failed to read magic number from MAC storage file");
        file.close();
        return false;
    }
    
    if (magicNumber != MAC_MAGIC_NUMBER) {
        Serial.println("Invalid magic number in MAC storage file");
        file.close();
        return false;
    }
    
    // Read number of MAC addresses
    uint8_t count;
    if (file.read(&count, sizeof(count)) != sizeof(count)) {
        Serial.println("Failed to read MAC count from storage file");
        file.close();
        return false;
    }
    
    // Read each MAC address
    for (uint8_t i = 0; i < count && i < MAX_MAC_ADDRESSES; i++) {
        uint8_t macLength;
        if (file.read(&macLength, sizeof(macLength)) != sizeof(macLength)) {
            Serial.println("Failed to read MAC length from storage file");
            break;
        }
        
        if (macLength > 18) { // MAC address shouldn't be longer than 17 chars + null terminator
            Serial.println("Invalid MAC length in storage file");
            break;
        }
        
        char macBuffer[19]; // 18 chars + null terminator
        if (file.read(reinterpret_cast<uint8_t*>(macBuffer), macLength) != macLength) {
            Serial.println("Failed to read MAC address from storage file");
            break;
        }
        
        macBuffer[macLength] = '\0'; // Ensure null termination
        macAddresses.push_back(String(macBuffer));
    }
    
    file.close();
    Serial.printf("Loaded %d MAC addresses from storage\n", macAddresses.size());
    return true;
}

bool MacAddressStorage::saveMacAddressesToFile() {
    File file = LittleFS.open(MAC_STORAGE_FILE, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open MAC storage file for writing");
        return false;
    }
    
    // Write magic number
    file.write(reinterpret_cast<const uint8_t*>(&MAC_MAGIC_NUMBER), sizeof(MAC_MAGIC_NUMBER));
    
    // Write number of MAC addresses
    uint8_t count = macAddresses.size();
    file.write(&count, sizeof(count));
    
    // Write each MAC address
    for (const String& macAddress : macAddresses) {
        uint8_t macLength = macAddress.length();
        file.write(&macLength, sizeof(macLength));
        file.write(reinterpret_cast<const uint8_t*>(macAddress.c_str()), macLength);
    }
    
    file.close();
    Serial.printf("Saved %d MAC addresses to storage\n", macAddresses.size());
    return true;
}

bool MacAddressStorage::isValidMacAddress(const String& macAddress) {
    // Basic MAC address validation (XX:XX:XX:XX:XX:XX format)
    if (macAddress.length() != 17) {
        return false;
    }
    
    for (int i = 0; i < 17; i++) {
        if (i % 3 == 2) {
            // Should be a colon
            if (macAddress.charAt(i) != ':') {
                return false;
            }
        } else {
            // Should be a hex digit
            char c = macAddress.charAt(i);
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                return false;
            }
        }
    }
    
    return true;
}

} // namespace NuggetsInc
