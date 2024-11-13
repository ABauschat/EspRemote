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

bool NFCLogic::readTag(uint8_t* uid, uint8_t* uidLength) {
    return nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, uidLength);
}

String NFCLogic::getTagType(uint8_t uidLength) {
    if (uidLength == 4) return TAG_TYPE_MIFARE_CLASSIC;
    if (uidLength == 7) return TAG_TYPE_NTAG2XX;
    switch (uidLength)
    {
    case 4:
        return TAG_TYPE_MIFARE_CLASSIC;

    case 7:
        return TAG_TYPE_NTAG2XX;

    case 10:
        return TAG_TYPE_MIFARE_ULTRALIGHT;

    default:
        return TAG_TYPE_UNKNOWN;
        break;
    }
    return TAG_TYPE_UNKNOWN;
}

String NFCLogic::readTagData(const String& tagType, uint8_t* uid, uint8_t uidLength) {
    if (tagType == TAG_TYPE_MIFARE_CLASSIC) {
        return readMIFAREClassic(uid, uidLength);
    }
    else if (tagType == TAG_TYPE_NTAG2XX) {
        return readNTAG2xx(uid, uidLength);
    }
    return "Data reading not supported for this tag type.";
}

bool NFCLogic::cloneTagData(const String& clonedTagType, uint8_t* targetUID, uint8_t targetUIDLength, const std::vector<String>& dataLines) {
    if (clonedTagType == TAG_TYPE_MIFARE_CLASSIC) {
        return writeMIFAREClassic(targetUID, targetUIDLength, dataLines);
    }
    else if (clonedTagType == TAG_TYPE_NTAG2XX) {
        return writeNTAG2xx(targetUID, targetUIDLength, dataLines);
    }
    return false;
}

bool NFCLogic::readAndParseTagData(TagData& tagData) {
    memset(tagData.uid, 0, sizeof(tagData.uid));
    tagData.uidLength = 0;

    if (!readTag(tagData.uid, &tagData.uidLength)) {
        return false; 
    }

    tagData.tagType = getTagType(tagData.uidLength);

    tagData.data = readTagData(tagData.tagType, tagData.uid, tagData.uidLength);

    if (tagData.tagType == TAG_TYPE_NTAG2XX) {
        std::vector<uint8_t> rawData;
        for (size_t i = 0; i < tagData.data.length(); i += 3) { 
            if (i + 2 > tagData.data.length()) break;
            String byteStr = tagData.data.substring(i, i + 2);
            rawData.push_back(static_cast<uint8_t>(strtol(byteStr.c_str(), NULL, 16)));
        }

        tagData.ndefRecords = parseNDEF(rawData.data(), rawData.size());

        tagData.availableSpace = "888 bytes";
    }
    else {
        tagData.availableSpace = "Unknown";
    }

    return true;
}

bool NFCLogic::validateAndCloneTag(const String& clonedTagType, std::vector<String>& dataLines) {
    uint8_t targetUID[7];
    uint8_t targetUIDLength = 0;

    if (!readTag(targetUID, &targetUIDLength)) {
        return false; 
    }

    String targetTagType = getTagType(targetUIDLength);

    if (clonedTagType != targetTagType) {
        return false; 
    }

    return cloneTagData(clonedTagType, targetUID, targetUIDLength, dataLines);
}

String NFCLogic::readMIFAREClassic(uint8_t* uid, uint8_t uidLen) {
    String dataStr = "";
    uint8_t sectorCount = 16;
    uint8_t blocksPerSector = 4;
    uint8_t defaultKeyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    for (uint8_t sector = 1; sector < sectorCount; sector++) { 
        for (uint8_t block = 0; block < blocksPerSector - 1; block++) { 
            uint8_t blockAddr = sector * blocksPerSector + block;
            uint8_t blockData[16];

            bool auth = (nfc.mifareclassic_AuthenticateBlock(uid, uidLen, blockAddr, MIFARE_CMD_AUTH_A, defaultKeyA) == 0x00);
            if (!auth) {
                dataStr += "Auth Failed: Sector " + String(sector) + ", Block " + String(blockAddr) + "\n";
                continue;
            }

            bool success = nfc.mifareclassic_ReadDataBlock(blockAddr, blockData);
            if (success) {
                for (uint8_t i = 0; i < 16; i++) {
                    char hexByte[3];
                    sprintf(hexByte, "%02X", blockData[i]);
                    dataStr += String(hexByte) + " ";
                }
                dataStr += "\n";
            }
            else {
                dataStr += "Read Failed: Sector " + String(sector) + ", Block " + String(blockAddr) + "\n";
            }
        }
    }
    return dataStr;
}

String NFCLogic::readNTAG2xx(uint8_t* uid, uint8_t uidLen) {
    String dataStr = "";
    for (uint8_t page = 4; page < 39; page++) {
        uint8_t pageData[4];
        if (nfc.ntag2xx_ReadPage(page, pageData)) {
            for (uint8_t i = 0; i < 4; i++) {
                char hexByte[3];
                sprintf(hexByte, "%02X", pageData[i]);
                dataStr += String(hexByte) + " ";
            }
            dataStr += "\n";
        }
        else {
            dataStr += "Read Failed: Page " + String(page) + "\n";
        }
    }
    return dataStr;
}

bool NFCLogic::writeMIFAREClassic(uint8_t* targetUID, uint8_t targetUIDLength, const std::vector<String>& dataLines) {
    uint8_t sectorCount = 16;
    uint8_t blocksPerSector = 4;
    uint8_t defaultKeyA[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    for (uint8_t sector = 1; sector < sectorCount; sector++) { 
        for (uint8_t block = 0; block < blocksPerSector - 1; block++) { 
            uint8_t blockAddr = sector * blocksPerSector + block;

            bool auth = (nfc.mifareclassic_AuthenticateBlock(targetUID, targetUIDLength, blockAddr, MIFARE_CMD_AUTH_A, defaultKeyA) == 0x00);
            if (!auth) {
                return false;
            }

            if (blockAddr >= dataLines.size()) {
                return false;
            }

            String dataLine = dataLines[blockAddr];
            uint8_t blockData[16];
            memset(blockData, 0, 16); 

            for (uint8_t i = 0; i < 16 && (i * 3 + 2) < dataLine.length(); i++) {
                String byteStr = dataLine.substring(i * 3, i * 3 + 2);
                blockData[i] = strtol(byteStr.c_str(), NULL, 16);
            }

            if (!writeAndVerifyMIFAREClassicBlock(blockAddr, blockData)) {
                return false;
            }
        }
    }
    return true;
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

bool NFCLogic::writeAndVerifyMIFAREClassicBlock(uint8_t blockAddr, const uint8_t* blockData) {
    int retryCount = 3;
    while (retryCount-- > 0) {
        if (nfc.mifareclassic_WriteDataBlock(blockAddr, const_cast<uint8_t*>(blockData))) {
            uint8_t verifyData[16];
            if (nfc.mifareclassic_ReadDataBlock(blockAddr, verifyData) && memcmp(verifyData, blockData, 16) == 0) {
                return true; 
            }
        }
        delay(500);
    }
    return false; 
}

std::vector<NDEFRecord> NFCLogic::parseNDEF(const uint8_t* data, size_t length) {
    std::vector<NDEFRecord> parsedRecords;
    size_t index = 0;

    while (index < length) {
        if (index + 2 > length) break; 

        uint8_t header = data[index];
        bool mb = header & 0x80;
        bool me = header & 0x40;
        bool cf = header & 0x20;
        bool sr = header & 0x10;
        bool il = header & 0x08;
        uint8_t tnflen = data[index + 1];
        size_t payloadLength = sr ? data[index + 2] : ((data[index + 2] << 8) | data[index + 3]);
        size_t typeOffset = sr ? 3 : 4;

        if (il) {
            typeOffset += 1;
        }

        if (index + typeOffset + tnflen > length) break; 

        String type = "";
        for (uint8_t i = 0; i < tnflen; i++) {
            type += (char)data[index + typeOffset + i];
        }

        size_t payloadOffset = typeOffset + tnflen + (il ? 1 : 0);
        if (payloadOffset > length) break;

        const uint8_t* payload = &data[index + payloadOffset];
        String parsedPayload = "";

        if (type == "U") {
            parsedPayload = parseURI(payload, payloadLength);
        }
        else if (type == "T") {
            parsedPayload = parseText(payload, payloadLength);
        }
        else {
            parsedPayload = "Unsupported Record Type";
        }

        NDEFRecord record;
        record.type = type;
        record.payload = parsedPayload;
        parsedRecords.push_back(record);

        if (sr) {
            index += 3 + tnflen + payloadLength + (il ? 1 : 0);
        }
        else {
            index += 4 + tnflen + payloadLength + (il ? 1 : 0);
        }

        if (me) break; 
    }

    return parsedRecords;
}

String NFCLogic::parseURI(const uint8_t* payload, size_t length) {
    if (length < 1) return "Invalid URI";

    uint8_t uriIdentifier = payload[0];
    String uri = "";

    const char* uriPrefixes[] = {
        "", "http://www.", "https://www.", "http://", "https://",
        "tel:", "mailto:", "ftp://anonymous:anonymous@", "ftp://ftp.",
        "ftps://", "sftp://", "smb://", "nfs://", "ftp://",
        "dav://", "news:", "telnet://", "imap:", "rtsp://",
        "urn:", "pop:", "sip:", "sips:", "tftp:", "btspp://",
        "btl2cap://", "btgoep://", "tcpobex://", "irdaobex://",
        "file://", "urn:epc:id:", "urn:epc:tag:", "urn:epc:pat:",
        "urn:epc:raw:", "urn:epc:", "urn:nfc:"
    };

    size_t prefixCount = sizeof(uriPrefixes) / sizeof(uriPrefixes[0]);
    if (uriIdentifier < prefixCount) {
        uri += uriPrefixes[uriIdentifier];
    }

    for (size_t i = 1; i < length; i++) {
        uri += (char)payload[i];
    }

    return uri;
}

String NFCLogic::parseText(const uint8_t* payload, size_t length) {
    if (length < 1) return "Invalid Text";

    uint8_t status = payload[0];
    bool isUTF16 = status & 0x80;
    uint8_t langLength = status & 0x3F;

    String text = "";
    for (size_t i = 1 + langLength; i < length; i++) {
        text += (char)payload[i];
    }

    return text;
}

} // namespace NuggetsInc