#ifndef MAC_ADDRESS_STORAGE_H
#define MAC_ADDRESS_STORAGE_H

#include <Arduino.h>
#include <vector>

namespace NuggetsInc {

class MacAddressStorage {
public:
    static MacAddressStorage& getInstance();
    
    // Prevent copying
    MacAddressStorage(const MacAddressStorage&) = delete;
    MacAddressStorage& operator=(const MacAddressStorage&) = delete;
    
    bool init();
    bool saveMacAddress(const String& macAddress);
    std::vector<String> getAllMacAddresses();
    bool clearAllMacAddresses();
    int getMacAddressCount();
    
    static const int MAX_MAC_ADDRESSES = 12;
    
private:
    MacAddressStorage();
    ~MacAddressStorage();
    
    bool loadMacAddresses();
    bool saveMacAddressesToFile();
    bool isValidMacAddress(const String& macAddress);
    
    std::vector<String> macAddresses;
    bool initialized;
    static const char* MAC_STORAGE_FILE;
    static const uint32_t MAC_MAGIC_NUMBER;
};

} // namespace NuggetsInc

#endif // MAC_ADDRESS_STORAGE_H
