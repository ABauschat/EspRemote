// CloneNFCState.cpp
#include "CloneNFCState.h"
#include "Device.h"
#include "EventManager.h"
#include "Application.h"
#include "StateFactory.h"
#include "Colors.h"
#include "DisplayUtils.h"
#include "Config.h"
#include <Wire.h>
#include <Adafruit_PN532.h>
#include "TagData.h"

namespace NuggetsInc
{

    CloneNFCState::CloneNFCState()
        : tagDetected(false),
        cloneTagData(false),
          displayUtils(nullptr)
    {

        nfcLogic = new NFCLogic(PN532_IRQ, PN532_RESET);
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
    }

    CloneNFCState::~CloneNFCState()
    {
        delete nfcLogic;
        delete displayUtils;
        delete currentTagData;
    }

    void CloneNFCState::onEnter()
    {
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
    }

    void CloneNFCState::onExit()
    {
    }

    void CloneNFCState::update()
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
            else if (event.type == EVENT_UP || event.type == EVENT_DOWN)
            {
                handleScroll(event.type);
            }
            else if (event.type == EVENT_LEFT || event.type == EVENT_RIGHT)
            {
            }

            else if (event.type == EVENT_SELECT)
            {
                if (tagDetected)
                {
                    
                    return;
                }
            }
        }

        if (!tagDetected)
        {
            readNFCTag();
        } else if (!cloneTagData) {
            //Display Tag Information and ask for confirmation
        } else if (cloneTagData) {
            //Write Data to New Tag
        }
    }

    void CloneNFCState::readNFCTag()
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
                // Play A Long Vibration to indicate an error
                displayUtils->displayMessage("Un-Supported Tag");
                delay(1500);
            } else {
                currentTagData = new TagData(NewtagData);
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

    bool CloneNFCState::cloneTag()
    {

        return true;
    }

    void CloneNFCState::handleScroll(EventType eventType)
    {
    }

} // namespace NuggetsInc
