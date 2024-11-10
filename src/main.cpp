#include <Arduino.h>
#include "Application.h"

using namespace NuggetsInc;

void setup() {
    Application::getInstance().init();
}

void loop() {
    Application::getInstance().run();
}
