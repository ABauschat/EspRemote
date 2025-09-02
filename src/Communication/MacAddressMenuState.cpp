#include "MacAddressMenuState.h"
#include "Device.h"
#include "StateFactory.h"
#include "Application.h"
#include "EventManager.h"
#include "Colors.h"
#include "MacAddressStorage.h"

namespace NuggetsInc {

MacAddressMenuState::MacAddressMenuState()
    : displayUtils(nullptr), selectedIndex(0), scrollOffset(0) {
    displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
}

MacAddressMenuState::~MacAddressMenuState() {
    delete displayUtils;
}

void MacAddressMenuState::onEnter() {
    loadMacAddresses();
    displayMenu();
}

void MacAddressMenuState::onExit() {
    // Clean up if needed
}

void MacAddressMenuState::update() {
    EventManager& eventManager = EventManager::getInstance();
    Event event;

    while (eventManager.getNextEvent(event)) {
        switch (event.type) {
            case EVENT_UP:
                scrollUp();
                displayMenu();
                break;
            case EVENT_DOWN:
                scrollDown();
                displayMenu();
                break;
            case EVENT_BACK:
            case EVENT_ACTION_TWO:
                Application::getInstance().changeState(StateFactory::createState(MENU_STATE));
                return;
            default:
                break;
        }
    }
}

void MacAddressMenuState::loadMacAddresses() {
    MacAddressStorage& macStorage = MacAddressStorage::getInstance();
    
    if (!macStorage.init()) {
        macAddresses.clear();
        macAddresses.push_back("Failed to load MAC addresses");
        return;
    }
    
    macAddresses = macStorage.getAllMacAddresses();
    
    if (macAddresses.empty()) {
        macAddresses.push_back("No MAC addresses stored");
    }
    
    // Reset selection if it's out of bounds
    if (selectedIndex >= macAddresses.size()) {
        selectedIndex = 0;
    }
}

void MacAddressMenuState::scrollUp() {
    if (selectedIndex > 0) {
        selectedIndex--;
        
        // Adjust scroll offset if needed
        if (selectedIndex < scrollOffset) {
            scrollOffset = selectedIndex;
        }
    }
}

void MacAddressMenuState::scrollDown() {
    if (selectedIndex < macAddresses.size() - 1) {
        selectedIndex++;
        
        // Adjust scroll offset if needed
        if (selectedIndex >= scrollOffset + MAX_VISIBLE_ITEMS) {
            scrollOffset = selectedIndex - MAX_VISIBLE_ITEMS + 1;
        }
    }
}

void MacAddressMenuState::displayMenu() {
    displayUtils->clearDisplay();
    
    // Display title
    displayUtils->setTextColor(COLOR_WHITE);
    displayUtils->setTextSize(2);
    displayUtils->setCursor(10, 10);
    displayUtils->print("Stored MAC Addresses");
    
    // Display count
    displayUtils->setTextSize(1);
    displayUtils->setCursor(10, 35);
    displayUtils->print("Count: " + String(macAddresses.size()));
    
    // Display MAC addresses
    displayUtils->setTextSize(1);
    int yPos = 55;
    int lineHeight = 20;
    
    for (int i = 0; i < MAX_VISIBLE_ITEMS && (scrollOffset + i) < macAddresses.size(); i++) {
        int macIndex = scrollOffset + i;
        
        // Highlight selected item
        if (macIndex == selectedIndex) {
            displayUtils->setTextColor(COLOR_ORANGE);
        } else {
            displayUtils->setTextColor(COLOR_WHITE);
        }
        
        displayUtils->setCursor(10, yPos + (i * lineHeight));
        
        // Display index and MAC address
        String displayText = String(macIndex + 1) + ": " + macAddresses[macIndex];
        
        // Truncate if too long for display
        if (displayText.length() > 25) {
            displayText = displayText.substring(0, 22) + "...";
        }
        
        displayUtils->print(displayText);
    }
    
    // Display scroll indicators if needed
    displayUtils->setTextColor(COLOR_WHITE);
    displayUtils->setTextSize(1);
    
    if (scrollOffset > 0) {
        displayUtils->setCursor(200, 55);
        displayUtils->print("^");
    }
    
    if (scrollOffset + MAX_VISIBLE_ITEMS < macAddresses.size()) {
        displayUtils->setCursor(200, 200);
        displayUtils->print("v");
    }
    
    // Display instructions
    displayUtils->setCursor(10, 220);
    displayUtils->print("UP/DOWN: Navigate  BACK: Return");
}

} // namespace NuggetsInc
