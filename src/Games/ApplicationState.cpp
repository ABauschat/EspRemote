#include "ApplicationState.h"
#include "Device.h"
#include "StateFactory.h"
#include "Application.h"
#include "EventManager.h"
#include "Colors.h"

namespace NuggetsInc
{

    ApplicationState::ApplicationState()
        : menuIndex(0),
          displayUtils(nullptr)
    {
        // Define menu options
        menu[0] = "Snake Game";

        // Initialize DisplayUtils
        displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
    }

    ApplicationState::~ApplicationState()
    {
        delete displayUtils;
    }

    void ApplicationState::onEnter()
    {
        displayMenu();
    }

    void ApplicationState::onExit()
    {
        // Clean up or reset if necessary
    }

    void ApplicationState::update()
    {
        EventManager &eventManager = EventManager::getInstance();
        Event event;

        while (eventManager.getNextEvent(event))
        {
            switch (event.type)
            {
            case EVENT_UP:
                menuIndex--;
                if (menuIndex < 0)
                    menuIndex = menuItems - 1; // Wrap to last menu item
                displayMenu();
                break;
            case EVENT_DOWN:
                menuIndex++;
                if (menuIndex >= menuItems)
                    menuIndex = 0; // Wrap to first menu item
                displayMenu();
                break;
            case EVENT_ACTION_ONE:
                executeSelection();
                break;
            case EVENT_ACTION_TWO:
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                break;
            default:
                break;
            }
        }
    }

    void ApplicationState::displayMenu()
    {
        Arduino_GFX *gfx = Device::getInstance().getDisplay();
        gfx->fillScreen(COLOR_BLACK);
        gfx->setTextSize(2);

        for (int i = 0; i < menuItems; i++)
        {
            if (i == menuIndex)
            {
                gfx->setTextColor(COLOR_ORANGE); // Highlight selected item
            }
            else
            {
                gfx->setTextColor(COLOR_WHITE);
            }
            gfx->setCursor(10, i * 30);
            gfx->println(menu[i].c_str());
        }
    }

    void ApplicationState::executeSelection()
    {
        Application &app = Application::getInstance();

        switch (menuIndex)
        {
        case 0: // Snake Game
            app.changeState(StateFactory::createState(SNAKE_GAME_STATE));
            break;
        default:
            // Handle unexpected cases gracefully
            break;
        }
    }

} // namespace NuggetsInc
