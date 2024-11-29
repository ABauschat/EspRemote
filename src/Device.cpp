// Device.cpp

#include "Device.h"
#include "Haptics.h"
#include "esp_sleep.h"

namespace NuggetsInc {

Device& Device::getInstance() {
    static Device instance;
    return instance;
}

Device::Device()
    : upPressed(false), downPressed(false), leftPressed(false),
      rightPressed(false), selectPressed(false), backPressed(false),
      actionOnePressed(false), actionTwoPressed(false),
      lastUpPressTime(0), lastDownPressTime(0), lastLeftPressTime(0),
      lastRightPressTime(0), lastSelectPressTime(0), lastBackPressTime(0),
      lastActionOnePressTime(0), lastActionTwoPressTime(0),
      lastBackButtonPressTime(0) {
    // Initialize the data bus and graphics objects
    bus = new Arduino_ESP32QSPI(6, 47, 18, 7, 48, 5);
    gfx = new Arduino_RM67162(bus, 17, 3);
}

void Device::init() {
    // Initialize serial communication for debugging
    Serial.begin(115200);
    delay(1000);
    Serial.println("Device initialization");

    // Initialize the display
    if (!gfx->begin()) {
        Serial.println("Display initialization failed!");
        while (1)
            ; // Stop execution if display fails to initialize
    }

    // Set maximum brightness
    setBrightness(255);

    // Initialize haptic feedback
    pinMode(VIBRATOR_PIN, OUTPUT);
    digitalWrite(VIBRATOR_PIN, LOW);

    // Initialize joystick buttons
    pinMode(JOY_UP_PIN, INPUT_PULLUP);
    pinMode(JOY_DOWN_PIN, INPUT_PULLUP);
    pinMode(JOY_LEFT_PIN, INPUT_PULLUP);
    pinMode(JOY_RIGHT_PIN, INPUT_PULLUP);
    pinMode(JOY_CENTER_PIN, INPUT_PULLUP);
    pinMode(SET_BUTTON_PIN, INPUT_PULLUP);
    pinMode(BACK_BUTTON_PIN, INPUT_PULLUP);
    pinMode(ACTION_ONE_BUTTON_PIN, INPUT_PULLUP);
    pinMode(ACTION_TWO_BUTTON_PIN, INPUT_PULLUP);

    // Start the haptics task
    Haptics::getInstance().begin();
}

void Device::startVibration() {
    digitalWrite(VIBRATOR_PIN, HIGH);
}

void Device::stopVibration() {
    digitalWrite(VIBRATOR_PIN, LOW);
}

Arduino_GFX* Device::getDisplay() {
    return gfx;
}

void Device::setBrightness(uint8_t value) {
    bus->beginWrite();
    bus->writeCommand(0x51); // Brightness control command
    bus->write(value);
    bus->endWrite();
}

void Device::playTone(uint32_t frequency, uint32_t duration) {
    // Initialize MCPWM GPIO
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PIEZO_PIN);

    // Configure MCPWM unit
    mcpwm_config_t pwm_config;
    pwm_config.frequency = frequency;
    pwm_config.cmpr_a = 50.0;
    pwm_config.cmpr_b = 0.0;
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;

    // Initialize MCPWM unit
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);

    // Wait for duration
    delay(duration);

    // Stop the PWM signal
    mcpwm_set_signal_low(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A);
}

void Device::update() {
    EventManager& eventManager = EventManager::getInstance();
    unsigned long currentTime = millis();

    // Macro to simplify debounce handling
    #define DEBOUNCE_BUTTON(pin, pressedFlag, lastPressTime, event) \
        if (digitalRead(pin) == LOW) { \
            if (!pressedFlag && (currentTime - lastPressTime >= debounceInterval)) { \
                pressedFlag = true; \
                lastPressTime = currentTime; \
                eventManager.queueEvent({event}); \
            } \
        } else { \
            pressedFlag = false; \
        }

    // Handle each button with debounce
    DEBOUNCE_BUTTON(JOY_UP_PIN, upPressed, lastUpPressTime, EVENT_UP);
    DEBOUNCE_BUTTON(JOY_DOWN_PIN, downPressed, lastDownPressTime, EVENT_DOWN);
    DEBOUNCE_BUTTON(JOY_LEFT_PIN, leftPressed, lastLeftPressTime, EVENT_LEFT);
    DEBOUNCE_BUTTON(JOY_RIGHT_PIN, rightPressed, lastRightPressTime, EVENT_RIGHT);
    DEBOUNCE_BUTTON(SET_BUTTON_PIN, selectPressed, lastSelectPressTime, EVENT_SELECT);
    DEBOUNCE_BUTTON(ACTION_ONE_BUTTON_PIN, actionOnePressed, lastActionOnePressTime, EVENT_ACTION_ONE);
    DEBOUNCE_BUTTON(ACTION_TWO_BUTTON_PIN, actionTwoPressed, lastActionTwoPressTime, EVENT_ACTION_TWO);

    // Special handling for BACK_BUTTON_PIN due to double press detection
    if (digitalRead(BACK_BUTTON_PIN) == LOW) {
        if (!backPressed && (currentTime - lastBackPressTime >= debounceInterval)) {
            backPressed = true;
            unsigned long now = millis();

            if (now - lastBackButtonPressTime < doublePressThreshold) {
                // Double press detected
                goToDeepSleep();
            } else {
                // Single press
                lastBackButtonPressTime = now;
                eventManager.queueEvent({EVENT_BACK});
            }
            lastBackPressTime = currentTime;
        }
    } else {
        backPressed = false;
    }

    #undef DEBOUNCE_BUTTON
}

void Device::goToDeepSleep() {
    // Perform necessary cleanup
    Haptics::getInstance().end();

    // Configure the wake-up source: BACK_BUTTON_PIN (e.g., GPIO0) on LOW level
    esp_sleep_enable_ext0_wakeup(static_cast<gpio_num_t>(BACK_BUTTON_PIN), 0);

    // Provide feedback before sleeping (optional)
    startVibration();
    delay(500);
    stopVibration();

    // Enter deep sleep
    esp_deep_sleep_start();

    // Code after esp_deep_sleep_start() will not be executed
}

} // namespace NuggetsInc
