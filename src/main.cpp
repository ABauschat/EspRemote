#include <Arduino.h>
#include "Application.h"
#include "esp_sleep.h"
#include "Haptics.h"

using namespace NuggetsInc;

void setup()
{
    Application::getInstance().init();
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();

    if (wakeup_reason == ESP_SLEEP_WAKEUP_EXT0)
    {
        Haptics::getInstance().doubleVibration();
    }
}

void loop()
{
    Application::getInstance().run();
}
