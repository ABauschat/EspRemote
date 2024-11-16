#include "SetupNFCDeviceState.h"
#include "StateFactory.h"
#include "Application.h"
#include "DisplayUtils.h"
#include "Colors.h"
#include "Config.h"
#include <Wire.h>

namespace NuggetsInc
{

    SetupNFCDeviceState::SetupNFCDeviceState()
        : nfcLogic(new NFCLogic(PN532_IRQ, PN532_RESET)),
          displayUtils(nullptr),
          macAddressFound(false),
          readingStarted(false),
          startTime(0) {}

    SetupNFCDeviceState::~SetupNFCDeviceState()
    {
        delete nfcLogic;
        nfcLogic = nullptr;
        macAddressFound = false;
        macAddress = "";
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
    }

    void SetupNFCDeviceState::onEnter()
    {
        Serial.begin(115200);
        while (!Serial)
        {
            ;
        }

        // Initialize DisplayUtils
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());

        displayUtils->newTerminalDisplay("Verifying NFC chip");

        if (!nfcLogic->initialize())
        {
            displayUtils->displayMessage("PN532 not found");
            delay(2000);
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            return;
        }

        displayUtils->addToTerminalDisplay("NFC module Found");
        tagDetected = false;

        // Display initial message
        displayUtils->newTerminalDisplay("Looking For Device...");

        // Define UART2 pins (assuming RX=40, TX=41)
        const int RX_PIN = 40;
        const int TX_PIN = 41;
        Serial2.begin(115200, SERIAL_8N1, RX_PIN, TX_PIN);

        // Initialize flags and variables
        macAddressFound = false;
        readingStarted = false;
        startTime = 0;

        // Clear any existing data in Serial2 buffer
        while (Serial2.available())
        {
            Serial2.read();
        }
    }

    void SetupNFCDeviceState::onExit()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
        Serial2.end();
    }

    void SetupNFCDeviceState::update()
    {
        EventManager &eventManager = EventManager::getInstance();
        Event event;

        while (eventManager.getNextEvent(event))
        {
            if (event.type == EVENT_BACK)
            {
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            }
            else if (event.type == EVENT_SELECT)
            {
                if (macAddressFound)
                {
                    cloningStarted = true;
                }
            }
        }

        if (!readingStarted && !macAddressFound)
        {
            readingStarted = true;
            displayUtils->addToTerminalDisplay("Scanning For Mac Adress...");
            startTime = millis();
            Serial2.println("GET_MAC");
        }

        if (readingStarted && !macAddressFound)
        {
            if (millis() - startTime >= 50)
            {
                readingStarted = false;
                return;
            }

            while (Serial2.available())
            {
                String response = Serial2.readStringUntil('\n');
                response.trim();

                if ((response.length() == 17 || response.length() == 18))
                {
                    macAddress = response;
                    macAddress.toUpperCase();

                    if (macAddress.length() == 18)
                    {
                        macAddress = macAddress.substring(1);
                    }

                    displayUtils->addToTerminalDisplay("Device Found!");
                    displayUtils->addToTerminalDisplay("MAC Address: " + macAddress);

                    macAddressFound = true;
                    readingStarted = false;
                    return;
                }
                else
                {
                    displayUtils->addToTerminalDisplay("Invalid Response: [" + response + "]");
                }
            }
        }

        if (macAddressFound && !cloningStarted)
        {
            displaySetupInstructions();
        }

        if (cloningStarted && !tagDetected)
        {
            readNFCTag();
        }

        if (tagDetected)
        {
            // Proceed to write tag data
            if (nfcLogic->writeTagData(*currentTagData))
            {
                displayUtils->displayMessage("Tag Cloned Successfully");
                delay(2000);
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            }
        }
    }

     void SetupNFCDeviceState::readNFCTag()
    {
        if (nfcLogic->isTagPresent())
        {
            displayUtils->displayMessage("NFC Tag Detected: Keep steady");

            TagData tag;
            const std::vector<uint8_t> &rawData = nfcLogic->readRawData();
            TagData NewtagData = tag.parseRawData(rawData);

            int validationCode = tag.ValidateTagData(NewtagData);

            if (validationCode != 0)
            {
                // Play a long vibration to indicate an error
                displayUtils->displayMessage("Un-Supported Tag");
                delay(1500);
                return;
            }
            else
            {
                currentTagData = new TagData(NewtagData);
                currentTagData->addTextRecord(std::string(macAddress.c_str()), "NI"); //WiteMac Adress With NI (Nuggetinc) Identifier
            }

            displayUtils->displayMessage("Verified NFC Tag");
            delay(1500);
            
            tagDetected = true;
        }
        else
        {
            displayUtils->displayMessage("Searching for NFC Tag");
            delay(100);
        }
    }

    void SetupNFCDeviceState::displaySetupInstructions()
    {
        displayUtils->displayMessage("Mac Adress Found:" + macAddress + "\n Press SELECT to Begin Transfer to NFC");
    }

} // namespace NuggetsInc
