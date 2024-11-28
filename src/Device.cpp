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

    // Read joystick inputs and generate events
    if (digitalRead(JOY_UP_PIN) == LOW && !upPressed) {
        upPressed = true;
        eventManager.queueEvent({EVENT_UP});
    } else if (digitalRead(JOY_UP_PIN) == HIGH) {
        upPressed = false;
    }

    if (digitalRead(JOY_DOWN_PIN) == LOW && !downPressed) {
        downPressed = true;
        eventManager.queueEvent({EVENT_DOWN});
    } else if (digitalRead(JOY_DOWN_PIN) == HIGH) {
        downPressed = false;
    }

    if (digitalRead(JOY_LEFT_PIN) == LOW && !leftPressed) {
        leftPressed = true;
        eventManager.queueEvent({EVENT_LEFT});
    } else if (digitalRead(JOY_LEFT_PIN) == HIGH) {
        leftPressed = false;
    }

    if (digitalRead(JOY_RIGHT_PIN) == LOW && !rightPressed) {
        rightPressed = true;
        eventManager.queueEvent({EVENT_RIGHT});
    } else if (digitalRead(JOY_RIGHT_PIN) == HIGH) {
        rightPressed = false;
    }

    if (digitalRead(SET_BUTTON_PIN) == LOW && !selectPressed) {
        selectPressed = true;
        eventManager.queueEvent({EVENT_SELECT});
    } else if (digitalRead(SET_BUTTON_PIN) == HIGH) {
        selectPressed = false;
    }

 if (digitalRead(BACK_BUTTON_PIN) == LOW && !backPressed) {
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
    } else if (digitalRead(BACK_BUTTON_PIN) == HIGH) {
        backPressed = false;
    }

    if (digitalRead(ACTION_ONE_BUTTON_PIN) == LOW && !actionOnePressed) {
        actionOnePressed = true;
        eventManager.queueEvent({EVENT_ACTION_ONE});
    } else if (digitalRead(ACTION_ONE_BUTTON_PIN) == HIGH) {
        actionOnePressed = false;
    }

    if (digitalRead(ACTION_TWO_BUTTON_PIN) == LOW && !actionTwoPressed) {
        actionTwoPressed = true;
        eventManager.queueEvent({EVENT_ACTION_TWO});
    } else if (digitalRead(ACTION_TWO_BUTTON_PIN) == HIGH) {
        actionTwoPressed = false;
    }
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
