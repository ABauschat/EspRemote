#include "SetupNFCDeviceState.h"
#include "StateFactory.h"
#include "Application.h"
#include "DisplayUtils.h"
#include "Colors.h"
#include "Config.h"
#include <Wire.h>
#include "Haptics.h"

namespace NuggetsInc
{

    SetupNFCDeviceState::SetupNFCDeviceState()
        : nfcLogic(new NFCLogic(PN532_IRQ, PN532_RESET)),
          displayUtils(nullptr),
          macAddressFound(false),
          readingStarted(false),
          cloningStarted(false),
          tagDetected(false),
          startTime(0),
          // --- initialize manual input members ---
          manualInputMode(false),
          manualInputCursor(0),
          manualMac("00:00:00:00:00:00")
    {}

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

        // --- Manual input mode: process events only if any input is made ---
        if (manualInputMode)
        {
            bool inputChanged = false;
            while (eventManager.getNextEvent(event))
            {
                // processManualInputEvent() will clear and update the display
                processManualInputEvent(event);
                inputChanged = true;
            }
            // If no input event occurred, simply return (leaving the display intact)
            if (inputChanged)
            {
                // (No additional clearing/update here.)
            }
            return; // Skip automatic MAC reading when in manual mode.
        }

        // --- Normal event processing when NOT in manual input mode ---
        while (eventManager.getNextEvent(event))
        {
            if (event.type == EVENT_BACK || event.type == EVENT_ACTION_TWO)
            {
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            }
            else if (event.type == EVENT_ACTION_ONE)
            {
                // If no MAC was found automatically, switch to manual input.
                if (!macAddressFound)
                {
                    manualInputMode = true;
                    // Reset manual input to default value and start at first digit.
                    manualMac = "00:00:00:00:00:00";
                    manualInputCursor = 0;
                    // Skip colon if needed.
                    if (manualMac.charAt(manualInputCursor) == ':')
                        manualInputCursor = 1;
                    // Clear display once for the new manual input session.
                    displayUtils->clearDisplay();
                    updateManualInputDisplay();
                    return;
                }
                // If a MAC was already found, then EVENT_ACTION_ONE begins cloning.
                else if (macAddressFound)
                {
                    displayUtils->clearDisplay();
                    cloningStarted = true;
                }
            }
        }

        // --- Automatic MAC reading (if not in manual input mode) ---
        if (!readingStarted && !macAddressFound)
        {
            readingStarted = true;
            displayUtils->displayMessage("Reading MAC Address. Connect Rx/Tx Pins");
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

                // If the response contains the keyword "MAC Address:", extract the substring after it.
                if (response.indexOf("MAC Address:") != -1)
                {
                    int keywordPos = response.indexOf("MAC Address:") + String("MAC Address:").length();
                    response = response.substring(keywordPos);
                    response.trim();
                }

                // Only accept responses of 17 or 18 characters that include colons.
                if ((response.length() == 17 || response.length() == 18) && response.indexOf(":") != -1)
                {
                    // Optionally, count colons to ensure proper format.
                    int colonCount = 0;
                    for (int i = 0; i < response.length(); i++)
                    {
                        if (response.charAt(i) == ':')
                        {
                            colonCount++;
                        }
                    }
                    if (colonCount != 5)
                    {
                        displayUtils->addToTerminalDisplay("Invalid MAC Format (colon count): [" + response + "]");
                        continue;
                    }

                    macAddress = response;
                    macAddress.toUpperCase();

                    // If length is 18, remove the extra character.
                    if (macAddress.length() == 18)
                    {
                        macAddress = macAddress.substring(1);
                    }

                    displayUtils->addToTerminalDisplay("Device Found!");
                    displayUtils->addToTerminalDisplay("MAC Address: " + macAddress);

                    Haptics::getInstance().doubleVibration();

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
            // Proceed to write tag data.
            if (nfcLogic->writeTagData(*currentTagData))
            {
                displayUtils->displayMessage("Tag Cloned Successfully");
                delay(2000);
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            }
        }
    }

    // ----- Manual Input Helper Methods -----

    // This routine now only prints the current state of manual input.
    // It does not clear the display on its own.
    void SetupNFCDeviceState::updateManualInputDisplay()
    {
        displayUtils->println("Enter MAC:");
        displayUtils->println(manualMac);

        // Build a caret line (a '^' under the active digit)
        String caretLine = "";
        for (int i = 0; i < manualMac.length(); i++)
        {
            caretLine += (i == manualInputCursor) ? "^" : " ";
        }
        displayUtils->println(caretLine);
    }

    // Process a single event while in manual input mode.
    // Each event handler clears the display and redraws the prompt immediately.
    void SetupNFCDeviceState::processManualInputEvent(const Event &event)
    {
        switch (event.type)
        {
            case EVENT_UP:
            {
                char curChar = manualMac.charAt(manualInputCursor);
                if ((curChar >= '0' && curChar <= '9') || (curChar >= 'A' && curChar <= 'F'))
                {
                    char newChar = incrementHexDigit(curChar);
                    manualMac.setCharAt(manualInputCursor, newChar);
                    displayUtils->clearDisplay();
                    updateManualInputDisplay();
                }
                break;
            }
            case EVENT_DOWN:
            {
                char curChar = manualMac.charAt(manualInputCursor);
                if ((curChar >= '0' && curChar <= '9') || (curChar >= 'A' && curChar <= 'F'))
                {
                    char newChar = decrementHexDigit(curChar);
                    manualMac.setCharAt(manualInputCursor, newChar);
                    displayUtils->clearDisplay();
                    updateManualInputDisplay();
                }
                break;
            }
            case EVENT_LEFT:
            {
                int newCursor = manualInputCursor - 1;
                while (newCursor >= 0 && manualMac.charAt(newCursor) == ':')
                    newCursor--;
                if (newCursor >= 0)
                {
                    manualInputCursor = newCursor;
                    displayUtils->clearDisplay();
                    updateManualInputDisplay();
                }
                break;
            }
            case EVENT_RIGHT:
            {
                int newCursor = manualInputCursor + 1;
                while (newCursor < manualMac.length() && manualMac.charAt(newCursor) == ':')
                    newCursor++;
                if (newCursor < manualMac.length())
                {
                    manualInputCursor = newCursor;
                    displayUtils->clearDisplay();
                    updateManualInputDisplay();
                }
                break;
            }
            case EVENT_ACTION_ONE:
            {
                // Finalize manual input.
                macAddress = manualMac;
                macAddress.toUpperCase();
                macAddressFound = true;
                manualInputMode = false;
                displayUtils->clearDisplay();
                displayUtils->addToTerminalDisplay("Device Found!");
                displayUtils->addToTerminalDisplay("MAC Address: " + macAddress);
                Haptics::getInstance().doubleVibration();
                break;
            }
            case EVENT_BACK:
            {
                // Cancel manual input mode.
                manualInputMode = false;
                displayUtils->displayMessage("Manual input canceled");
                delay(1000);
                break;
            }
            default:
                break;
        }
    }

    // Increment a hexadecimal digit (wraps from 'F' to '0').
    char SetupNFCDeviceState::incrementHexDigit(char digit)
    {
        if (digit >= '0' && digit <= '8')
            return digit + 1;
        if (digit == '9')
            return 'A';
        if (digit >= 'A' && digit <= 'E')
            return digit + 1;
        if (digit == 'F')
            return '0';
        return digit;
    }

    // Decrement a hexadecimal digit (wraps from '0' to 'F').
    char SetupNFCDeviceState::decrementHexDigit(char digit)
    {
        if (digit >= '1' && digit <= '9')
            return digit - 1;
        if (digit == '0')
            return 'F';
        if (digit >= 'B' && digit <= 'F')
            return digit - 1;
        if (digit == 'A')
            return '9';
        return digit;
    }

    // -------------------------------
    void SetupNFCDeviceState::displaySetupInstructions()
    {
        displayUtils->displayMessage("Mac Address Found: " + macAddress +
                                     "\n Press SELECT to Begin Transfer to NFC");
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
                displayUtils->displayMessage("Un-Supported Tag");
                delay(1500);
                return;
            }
            else
            {
                currentTagData = new TagData(NewtagData);
                currentTagData->addTextRecord(std::string(macAddress.c_str()), "NI");
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

} // namespace NuggetsInc
