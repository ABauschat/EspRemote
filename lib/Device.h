#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>
#include <Arduino_GFX_Library.h>
#include "driver/mcpwm.h"
#include "EventManager.h"
#include "esp_sleep.h" // Include ESP32 sleep functions

namespace NuggetsInc {

class Device {
public:
    static Device& getInstance();

    // Prevent copying
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    // Initialize hardware
    void init();

    // Accessors for hardware components
    Arduino_GFX* getDisplay();
    void setBrightness(uint8_t value);
    void playTone(uint32_t frequency, uint32_t duration);

    void startVibration();
    void stopVibration();

    // Update device state and generate events
    void update();

    // Input pins
    static const int JOY_UP_PIN = 1;
    static const int JOY_DOWN_PIN = 2;
    static const int JOY_LEFT_PIN = 3;
    static const int JOY_RIGHT_PIN = 10;
    static const int JOY_CENTER_PIN = 11;

    static const int SET_BUTTON_PIN = 21;
    static const int BACK_BUTTON_PIN = 0;

    static const int ACTION_ONE_BUTTON_PIN = 13;
    static const int ACTION_TWO_BUTTON_PIN = 12;

    // Piezo buzzer pin
    static const int PIEZO_PIN = 16;

    // Vibrator motor pin
    static const int VIBRATOR_PIN = 44;

    // IR receiver pin Input
    static const int IR_RECEIVER_PIN = 15;

    // IR transmitter pin Output
    static const int IR_TRANSMITTER_PIN = 14;

private:
    Device(); // Private constructor
    Arduino_DataBus* bus;
    Arduino_GFX* gfx;

    // Input state tracking
    bool upPressed;
    bool downPressed;
    bool leftPressed;
    bool rightPressed;
    bool selectPressed;
    bool backPressed;

    // Variables for long press detection
    unsigned long backButtonPressTime;
    bool backButtonLongPressDetected;
};

} // namespace NuggetsInc

#endif // DEVICE_H
