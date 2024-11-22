// RemoteControlState.cpp
#include "RemoteControlState.h"
#include "Application.h"
#include "Device.h"

namespace NuggetsInc
{
    RemoteControlState *RemoteControlState::activeInstance = nullptr;

    RemoteControlState::RemoteControlState(uint8_t *macAddress)
        : isPeerAdded(false), displayUtils(nullptr)
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

        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }
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

            struct_message outgoingMessage;
            outgoingMessage.messageID = millis();
            strcpy(outgoingMessage.messageType, "command");
            strcpy(outgoingMessage.command, "Boop");
            strcpy(outgoingMessage.data, "");
            esp_now_send(device2MAC, (uint8_t *)&outgoingMessage, sizeof(outgoingMessage));
            displayUtils->clearDisplay();
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
        outgoingMessage.messageID = millis();
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
        // Optional: Handle send status if needed
    }

    RemoteControlState::CommandType RemoteControlState::mapCommandStringToEnum(const char *command)
    {
        if (strcmp(command, "CLEAR_DISPLAY") == 0)
            return CommandType::CLEAR_DISPLAY;
        if (strcmp(command, "DISPLAY_MESSAGE") == 0)
            return CommandType::DISPLAY_MESSAGE;
        if (strcmp(command, "NEW_TERMINAL_DISPLAY") == 0)
            return CommandType::NEW_TERMINAL_DISPLAY;
        if (strcmp(command, "ADD_TO_TERMINAL_DISPLAY") == 0)
            return CommandType::ADD_TO_TERMINAL_DISPLAY;
        if (strcmp(command, "PRINTLN") == 0)
            return CommandType::PRINTLN;
        if (strcmp(command, "PRINT") == 0)
            return CommandType::PRINT;
        if (strcmp(command, "SET_CURSOR") == 0)
            return CommandType::SET_CURSOR;
        if (strcmp(command, "SET_TEXT_SIZE") == 0)
            return CommandType::SET_TEXT_SIZE;
        if (strcmp(command, "SET_TEXT_COLOR") == 0)
            return CommandType::SET_TEXT_COLOR;
        if (strcmp(command, "FILL_SCREEN") == 0)
            return CommandType::FILL_SCREEN;
        if (strcmp(command, "DRAW_RECT") == 0)
            return CommandType::DRAW_RECT;
        if (strcmp(command, "FILL_RECT") == 0)
            return CommandType::FILL_RECT;
        return CommandType::UNKNOWN;
    }

    void RemoteControlState::handleOnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int len)
    {
        struct_message incomingMessage;
        if (len >= sizeof(struct_message))
        {
            memcpy(&incomingMessage, incomingData, sizeof(struct_message));

            // Ensure null-termination
            incomingMessage.messageType[sizeof(incomingMessage.messageType) - 1] = '\0';
            incomingMessage.command[sizeof(incomingMessage.command) - 1] = '\0';
            incomingMessage.data[sizeof(incomingMessage.data) - 1] = '\0';

            if (strcmp(incomingMessage.messageType, "command") == 0)
            {
                Serial.print("Received command: ");
                Serial.println(incomingMessage.command);

                CommandType cmdType = mapCommandStringToEnum(incomingMessage.command);

                switch (cmdType)
                {
                case CommandType::CLEAR_DISPLAY:
                    handleClearDisplay();
                    break;
                case CommandType::DISPLAY_MESSAGE:
                    if (strlen(incomingMessage.data) > 0)
                        handleDisplayMessage(String(incomingMessage.data));
                    break;
                case CommandType::NEW_TERMINAL_DISPLAY:
                    if (strlen(incomingMessage.data) > 0)
                        handleNewTerminalDisplay(String(incomingMessage.data));
                    break;
                case CommandType::ADD_TO_TERMINAL_DISPLAY:
                    if (strlen(incomingMessage.data) > 0)
                        handleAddToTerminalDisplay(String(incomingMessage.data));
                    break;
                case CommandType::PRINTLN:
                    if (strlen(incomingMessage.data) > 0)
                        handlePrintln(String(incomingMessage.data));
                    break;
                case CommandType::PRINT:
                    if (strlen(incomingMessage.data) > 0)
                        handlePrint(String(incomingMessage.data));
                    break;
                case CommandType::SET_CURSOR:
                    handleSetCursor(incomingMessage.data);
                    break;
                case CommandType::SET_TEXT_SIZE:
                    handleSetTextSize(incomingMessage.data);
                    break;
                case CommandType::SET_TEXT_COLOR:
                    handleSetTextColor(incomingMessage.data);
                    break;
                case CommandType::FILL_SCREEN:
                    handleFillScreen(incomingMessage.data);
                    break;
                case CommandType::DRAW_RECT:
                    handleDrawRect(incomingMessage.data);
                    break;
                case CommandType::FILL_RECT:
                    handleFillRect(incomingMessage.data);
                    break;
                case CommandType::UNKNOWN:
                default:
                    Serial.print("Unknown command received: ");
                    Serial.println(incomingMessage.command);
                    break;
                }

                // Send ACK back to sender
                struct_message ackMessage;
                ackMessage.messageID = incomingMessage.messageID; // Copy the messageID
                strcpy(ackMessage.messageType, "ack");
                ackMessage.command[0] = '\0';
                ackMessage.data[0] = '\0';

                esp_err_t result = esp_now_send(mac_addr, (uint8_t *)&ackMessage, sizeof(ackMessage));
                if (result != ESP_OK)
                {
                    Serial.println("Failed to send ACK");
                }
                else
                {
                    Serial.println("ACK sent");
                }
            }
            else
            {
                displayUtils->displayMessage("Unknown message type received.");
            }
        }
        else
        {
            displayUtils->displayMessage("Received invalid message");
        }
    }

    // Command handler implementations

    void RemoteControlState::handleClearDisplay()
    {
        displayUtils->clearDisplay();
    }

    void RemoteControlState::handleDisplayMessage(const String &message)
    {
        displayUtils->displayMessage(message);
    }

    void RemoteControlState::handleNewTerminalDisplay(const String &message)
    {
        displayUtils->newTerminalDisplay(message);
    }

    void RemoteControlState::handleAddToTerminalDisplay(const String &message)
    {
        displayUtils->addToTerminalDisplay(message);
    }

    void RemoteControlState::handlePrintln(const String &message)
    {
        displayUtils->println(message);
    }

    void RemoteControlState::handlePrint(const String &message)
    {
        displayUtils->print(message);
    }

    void RemoteControlState::handleSetCursor(const char *data)
    {
        int x, y;
        if (sscanf(data, "%d,%d", &x, &y) == 2)
        {
            displayUtils->setCursor(x, y);
        }
        else
        {
            displayUtils->displayMessage("Invalid SET_CURSOR data");
        }
    }

    void RemoteControlState::handleSetTextSize(const char *data)
    {
        int size;
        if (sscanf(data, "%d", &size) == 1 && size > 0)
        {
            displayUtils->setTextSize(static_cast<uint8_t>(size));
        }
        else
        {
            displayUtils->displayMessage("Invalid SET_TEXT_SIZE data");
        }
    }

    void RemoteControlState::handleSetTextColor(const char *data)
    {
        int color;
        if (sscanf(data, "%d", &color) == 1)
        {
            displayUtils->setTextColor(static_cast<uint16_t>(color));
        }
        else
        {
            displayUtils->displayMessage("Invalid SET_TEXT_COLOR data");
        }
    }

    void RemoteControlState::handleFillScreen(const char *data)
    {
        int color;
        if (sscanf(data, "%d", &color) == 1)
        {
            displayUtils->fillScreen(static_cast<uint16_t>(color));
        }
        else
        {
            displayUtils->displayMessage("Invalid FILL_SCREEN data");
        }
    }

    void RemoteControlState::handleDrawRect(const char *data)
    {
        int x, y, w, h, color;
        if (sscanf(data, "%d,%d,%d,%d,%d", &x, &y, &w, &h, &color) == 5)
        {
            displayUtils->drawRect(x, y, w, h, static_cast<uint16_t>(color));
        }
        else
        {
            displayUtils->displayMessage("Invalid DRAW_RECT data");
        }
    }

    void RemoteControlState::handleFillRect(const char *data)
    {
        int x, y, w, h, color;
        if (sscanf(data, "%d,%d,%d,%d,%d", &x, &y, &w, &h, &color) == 5)
        {
            displayUtils->fillRect(x, y, w, h, static_cast<uint16_t>(color));
        }
        else
        {
            displayUtils->displayMessage("Invalid FILL_RECT data");
        }
    }
} // namespace NuggetsInc
