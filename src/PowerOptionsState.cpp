#include "PowerOptionsState.h"
#include "DisplayUtils.h"
#include "Device.h"

#define VOLTAGE_DIVIDER_FACTOR 2.0 // Update based on your voltage divider
#define UPDATE_INTERVAL 500       // Update interval in milliseconds

namespace NuggetsInc
{
    PowerOptionsState::PowerOptionsState()
        : lastUpdateTime(0) {}

    PowerOptionsState::~PowerOptionsState() {}

    void PowerOptionsState::onEnter()
    {
        Serial.println("Entering PowerOptionsState.");
        pinMode(PIN_BAT_VOLT, INPUT);
        lastUpdateTime = millis(); // Initialize the timer
    }

    void PowerOptionsState::onExit()
    {
        Serial.println("Exiting PowerOptionsState.");
    }

    void PowerOptionsState::update()
    {
        // Check if the update interval has passed
        unsigned long currentTime = millis();
        if (currentTime - lastUpdateTime >= UPDATE_INTERVAL)
        {
            lastUpdateTime = currentTime; // Reset the timer

            // Read the raw ADC value
            int rawValue = analogRead(PIN_BAT_VOLT);
            Serial.println("Raw ADC Value: " + String(rawValue));

            // Convert raw ADC value to voltage
            float batteryVoltage = rawValue * (3.3 / 4095.0) * VOLTAGE_DIVIDER_FACTOR;
            Serial.println("Battery Voltage: " + String(batteryVoltage, 2) + "V");

            // Calculate battery percentage
            float batteryPercentage = calculateBatteryPercentage(batteryVoltage);
            Serial.println("Battery Percentage: " + String(batteryPercentage, 1) + "%");

            // Display battery percentage
            DisplayUtils *displayUtils = new DisplayUtils(Device::getInstance().getDisplay());
            displayUtils->clearDisplay();
            displayUtils->setTextSize(2);
            displayUtils->setTextColor(COLOR_WHITE);
            displayUtils->setCursor(10, 10);
            displayUtils->println("Battery: " + String(batteryPercentage, 1) + "%");

            delete displayUtils; // Clean up dynamically allocated memory
        }
    }

    float PowerOptionsState::calculateBatteryPercentage(float voltage)
    {
        const float MIN_VOLTAGE = 3.0; // Adjust based on your battery
        const float MAX_VOLTAGE = 4.2;

        if (voltage <= MIN_VOLTAGE)
        {
            return 0.0;
        }
        else if (voltage >= MAX_VOLTAGE)
        {
            return 100.0;
        }
        else
        {
            return ((voltage - MIN_VOLTAGE) / (MAX_VOLTAGE - MIN_VOLTAGE)) * 100.0;
        }
    }

} // namespace NuggetsInc
