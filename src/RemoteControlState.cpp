#include "RemoteControlState.h"
#include "Application.h"
#include "Device.h"

namespace NuggetsInc
{
    RemoteControlState *RemoteControlState::activeInstance = nullptr;

    RemoteControlState::RemoteControlState(uint8_t *macAddress)
        : isPeerAdded(false)
    {
        if (macAddress)
        {
            memcpy(device2MAC, macAddress, sizeof(device2MAC));
        }
        else
        {
            memset(device2MAC, 0, sizeof(device2MAC));
        }
    }

    RemoteControlState::~RemoteControlState()
    {
        if (isPeerAdded)
        {
            esp_now_del_peer(device2MAC);
        }

        esp_now_deinit();
        WiFi.disconnect();
    }

    void RemoteControlState::onEnter()
    {
        activeInstance = this;
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());

        setupESPNow();
    }

    void RemoteControlState::onExit()
    {
        activeInstance = nullptr;
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

            if (isPeerAdded)
            {
                handleInput(event.type);
            }
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

        esp_now_register_send_cb(onDataSentCallback);
        esp_now_register_recv_cb(onDataRecvCallback);

        esp_now_peer_info_t peerInfo = {};
        memcpy(peerInfo.peer_addr, device2MAC, sizeof(device2MAC));
        peerInfo.channel = 0;
        peerInfo.encrypt = false;

        if (esp_now_add_peer(&peerInfo) == ESP_OK)
        {
            isPeerAdded = true;
            displayUtils->displayMessage("Peer added successfully");
        }
        else
        {
            displayUtils->displayMessage("Error: Failed to add peer");
            delay(1000);
            Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
        }
    }

    void RemoteControlState::handleInput(EventType eventType)
    {
        if (!isPeerAdded)
        {
            displayUtils->displayMessage("Peer not connected");
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

        if (esp_now_send(device2MAC, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage)) != ESP_OK)
        {
            displayUtils->displayMessage("Failed to send command");
        }
    }

    void RemoteControlState::onDataSentCallback(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
        if (activeInstance)
        {
            activeInstance->handleOnDataSent(mac_addr, status);
        }
    }

    void RemoteControlState::onDataRecvCallback(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
    {
        if (activeInstance)
        {
            activeInstance->handleOnDataRecv(mac_addr, incomingData, len);
        }
    }

    void RemoteControlState::handleOnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
    {
        // Do Nothing for Now
    }

    void RemoteControlState::handleOnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
    {
        struct_message incomingMessage;
        if (len >= sizeof(struct_message))
        {
            memcpy(&incomingMessage, incomingData, sizeof(incomingMessage));

            if (strcmp(incomingMessage.messageType, "response") == 0)
            {
                displayUtils->displayMessage("Received: " + String(incomingMessage.command));
            } 
        }
        else
        {
            displayUtils->displayMessage("Received invalid message");
        }
    }
} // namespace NuggetsInc