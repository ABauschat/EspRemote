// RemoteControlState.cpp - Simplified with RemoteService
#include "RemoteControlState.h"
#include "RemoteService.h"
#include "Application.h"
#include "Device.h"
#include "MessageTypes.h"
#include "MacAddressStorage.h"
#include <Arduino.h>


namespace NuggetsInc
{
    RemoteControlState *RemoteControlState::activeInstance = nullptr;

    RemoteControlState::RemoteControlState(uint8_t *macAddress)
        : displayUtils(nullptr), remoteService_(nullptr)
    {
        // Save the MAC address of the target device
        if (macAddress)
        {
            memcpy(device2MAC, macAddress, sizeof(device2MAC));
        }
        else
        {
            memset(device2MAC, 0, sizeof(device2MAC));
        }

        remoteService_ = new RemoteService();
    }

    RemoteControlState::~RemoteControlState()
    {
        if (displayUtils)
        {
            delete displayUtils;
            displayUtils = nullptr;
        }

        if (remoteService_)
        {
            delete remoteService_;
            remoteService_ = nullptr;
        }

        if (activeInstance == this)
        {
            activeInstance = nullptr;
        }
    }

    void RemoteControlState::onEnter()
    {
        activeInstance = this;
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());

        // Initialize RemoteService with target MAC
        if (remoteService_ && remoteService_->begin(device2MAC)) {
            // Send BOOOP to Notify target
            remoteService_->sendCommand(CMD_BOOOP);
        } 
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
            handleInput(event.type);
        }
    }

    void RemoteControlState::handleInput(EventType eventType)
    {
        if (!remoteService_)
        {
            displayUtils->displayMessage("Internal Error: RemoteService not initialized");
            return;
        }

        uint8_t commandID = 0;
        switch (eventType)
        {
        case EVENT_UP:
            commandID = CMD_MOVE_UP;
            break;
        case EVENT_DOWN:
            commandID = CMD_MOVE_DOWN;
            break;
        case EVENT_LEFT:
            commandID = CMD_MOVE_LEFT;
            break;
        case EVENT_RIGHT:
            commandID = CMD_MOVE_RIGHT;
            break;
        case EVENT_ACTION_ONE:
            commandID = CMD_SELECT;
            break;
        case EVENT_ACTION_TWO:
            commandID = CMD_BACK;
            break;
        default:
            return;
        }

        // Send command via RemoteService
        remoteService_->sendCommandNonBlocking(commandID);
    }

    void RemoteControlState::handleSyncNodes(const char *data)
    {
        MacAddressStorage &macStorage = MacAddressStorage::getInstance();
        String macAddress = String(data);
        macStorage.saveMacAddress(macAddress);
    }

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

    void RemoteControlState::handleBeginPlot(const char *data)
    {
        int minX, maxX, minY, maxY;
        char xTitle[50], yTitle[50];
        if (sscanf(data, "%d,%d,%d,%d,%49[^,],%49[^,]", &minX, &maxX, &minY, &maxY, xTitle, yTitle) == 6)
        {
            displayUtils->beginPlot(String(xTitle), String(yTitle), minX, maxX, minY, maxY);
        }
        else
        {
            displayUtils->displayMessage("Invalid BEGIN_PLOT data");
        }
    }

    void RemoteControlState::handlePlotPoint(const char *data)
    {
        int xValue, yValue, color;
        if (sscanf(data, "%d,%d, %d", &xValue, &yValue, &color) == 3)
        {
            displayUtils->plotPoint(xValue, yValue, static_cast<uint16_t>(color));
        }
        else
        {
            displayUtils->displayMessage("Invalid PLOT_POINT data");
        }
    }

} // namespace NuggetsInc
