#include "RemoteControlState.h"
#include "StateFactory.h"
#include "Application.h"
#include "DisplayUtils.h"
#include "Config.h"
#include <esp_now.h>
#include <WiFi.h>

namespace NuggetsInc
{
    // Initialize the static pointer to nullptr
    RemoteControlState *RemoteControlState::activeInstance = nullptr;

    RemoteControlState::RemoteControlState()
        : nfcLogic(new NFCLogic(PN532_IRQ, PN532_RESET)),
          isPeerAdded(false)
    {}

    RemoteControlState::~RemoteControlState()
    {
        delete nfcLogic;
        delete displayUtils;

        if (isPeerAdded)
        {
            esp_now_del_peer(device2MAC);
        }

        esp_now_deinit();
        WiFi.disconnect();
        displayUtils = nullptr;
        activeInstance = nullptr;
        isPeerAdded = false;
        memset(device2MAC, 0, sizeof(device2MAC));
    }

    void RemoteControlState::onEnter()
    {
        activeInstance = this; // Set the static instance pointer
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
        displayUtils->clearDisplay();

        tagDetected = false;

        setupESPNow();
        displayRemoteControlInterface();
    }

    void RemoteControlState::onExit()
    {
        activeInstance = nullptr; // Clear the static instance pointer
    }

    void RemoteControlState::update()
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
            else
            {
                if (isPeerAdded)
                {
                    handleInput(event.type);
                }
            }
        }

        if (!tagDetected)
        {
            readNFCTag();
        } 
    }

void RemoteControlState::readNFCTag()
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
        }

        displayUtils->displayMessage("Verified NFC Tag");
        delay(500);

        uint8_t* macAddress = currentTagData->CheckForTextRecordWithNITag();

        if(macAddress == nullptr)
        {
            displayUtils->displayMessage("No MAC Address found");
            delay(1500);
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
            return;
        }
        else
        {
            memcpy(device2MAC, macAddress, sizeof(device2MAC));
            displayUtils->addToTerminalDisplay("Mac Address Found");
            delay(500);

            //print mac adress

            displayUtils->addToTerminalDisplay("MAC Address: ");
            for (int i = 0; i < 6; i++)
            {
                displayUtils->addToTerminalDisplay(String(device2MAC[i], HEX));
            }

            delay(9000);
            displayUtils->clearDisplay();
        }

        setupESPNow();
        tagDetected = true;
    }
    else
    {
        displayUtils->displayMessage("Searching for NFC Tag");
        delay(100);
    }
}


    void RemoteControlState::handleInput(EventType eventType)
    {
        if (!isPeerAdded)
        {
            displayUtils->addToTerminalDisplay("Peer not connected");
            return;
        }

        struct_message outgoingMessage;
        strcpy(outgoingMessage.messageType, "command");

        switch (eventType)
        {
        case EVENT_UP:
            strcpy(outgoingMessage.command, "MOVE_UP");
            break;
        case EVENT_DOWN:
            strcpy(outgoingMessage.command, "MOVE_DOWN");
            break;
        case EVENT_LEFT:
            strcpy(outgoingMessage.command, "MOVE_LEFT");
            break;
        case EVENT_RIGHT:
            strcpy(outgoingMessage.command, "MOVE_RIGHT");
            break;
        case EVENT_SELECT:
            strcpy(outgoingMessage.command, "SELECT");
            break;
        default:
            return;
        }

        // Send the command to Device 2 using its MAC address
        if (esp_now_send(device2MAC, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage)) != ESP_OK)
        {
            displayUtils->addToTerminalDisplay("Failed to send command");
        }
    }

    void RemoteControlState::setupESPNow()
    {
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();

        if (esp_now_init() != ESP_OK)
        {
            displayUtils->displayMessage("ESP-NOW init failed");
            return;
        }

        // Register send callback
        esp_now_register_send_cb(onDataSentCallback);

        // Register receive callback
        esp_now_register_recv_cb(onDataRecvCallback);

        // Add Device 2 as a peer
        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, device2MAC, sizeof(device2MAC));
        peerInfo.channel = 0; 
        peerInfo.encrypt = false;

        if (esp_now_add_peer(&peerInfo) == ESP_OK)
        {
            isPeerAdded = true;
            displayUtils->addToTerminalDisplay("Peer added successfully");
        }
        else
        {
            displayUtils->addToTerminalDisplay("Failed to add peer");
        }
    }

    // Static send callback
    void RemoteControlState::onDataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
        if (activeInstance)
        {
            activeInstance->handleOnDataSent(mac_addr, status);
        }
    }

    // Static receive callback
    void RemoteControlState::onDataRecvCallback(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
    {
        if (activeInstance)
        {
            activeInstance->handleOnDataRecv(mac_addr, incomingData, len);
        }
    }

    void RemoteControlState::handleOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
        String result = (status == ESP_NOW_SEND_SUCCESS) ? "Send success" : "Send failed";
        displayUtils->addToTerminalDisplay(result);
    }

    void RemoteControlState::handleOnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
    {
        struct_message incomingMessage;
        if (len >= sizeof(struct_message))
        {
            memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));

            if (strcmp(incomingMessage.messageType, "response") == 0)
            {
                String response = String(incomingMessage.command);
                displayUtils->addToTerminalDisplay("Received: " + response);
            }
        }
        else
        {
            displayUtils->addToTerminalDisplay("Received invalid message");
        }
    }

    void RemoteControlState::displayRemoteControlInterface()
    {

    }

} // namespace NuggetsInc
