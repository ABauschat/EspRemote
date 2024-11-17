#include "EnterRemoteControlState.h"
#include "StateFactory.h"
#include "Application.h"
#include "RemoteControlState.h"
#include "Config.h"

namespace NuggetsInc
{
    EnterRemoteControlState *EnterRemoteControlState::activeInstance = nullptr;

    EnterRemoteControlState::EnterRemoteControlState()
        : nfcLogic(new NFCLogic(PN532_IRQ, PN532_RESET)) {}

    EnterRemoteControlState::~EnterRemoteControlState()
    {
        delete nfcLogic;
        delete displayUtils;
    }

    void EnterRemoteControlState::onEnter()
    {
        activeInstance = this;
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
    }

    void EnterRemoteControlState::onExit()
    {
        activeInstance = nullptr;
    }

    void EnterRemoteControlState::update()
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
        }

        readNFCTag();
    }

    void EnterRemoteControlState::readNFCTag()
    {
        if (nfcLogic->isTagPresent())
        {
            displayUtils->displayMessage("NFC Tag Detected: Keep steady");

            TagData tag;
            const std::vector<uint8_t> &rawData = nfcLogic->readRawData();
            TagData newTagData = tag.parseRawData(rawData);

            if (tag.ValidateTagData(newTagData) != 0)
            {
                displayUtils->displayMessage("Unsupported Tag");
                delay(1500);
                return;
            }

            currentTagData = new TagData(newTagData);
            uint8_t *macAddress = currentTagData->CheckForTextRecordWithNITag();

            if (!macAddress)
            {
                displayUtils->displayMessage("No MAC Address found");
                delay(1000);
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            }

            memcpy(device2MAC, macAddress, sizeof(device2MAC));
            displayUtils->displayMessage("MAC Address Found");
            delay(500);

            Application::getInstance().changeState(new RemoteControlState(device2MAC));
        }
        else
        {
            displayUtils->displayMessage("Please Scan NFC Tag To Connect to a Device");
            delay(100);
        }
    }
} // namespace NuggetsInc
