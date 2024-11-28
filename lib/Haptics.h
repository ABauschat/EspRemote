#ifndef HAPTICS_H
#define HAPTICS_H

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>

namespace NuggetsInc {

class Haptics {
public:
    static Haptics& getInstance();

    // Prevent copying
    Haptics(const Haptics&) = delete;
    Haptics& operator=(const Haptics&) = delete;

    // Haptic functions
    void singleVibration();
    void doubleVibration();

    // Start and stop the haptics task
    void begin();
    void end();

private:
    Haptics(); // Private constructor
    static void hapticsTask(void* parameter);

    // Task handle for the haptics task
    TaskHandle_t hapticsTaskHandle;

    // Queue handle for communication
    QueueHandle_t hapticsQueue;

    // Vibration patterns
    enum VibrationPattern {
        NONE,
        SINGLE,
        DOUBLE
    };

    // Vibration command structure
    struct VibrationCommand {
        VibrationPattern pattern;
    };
};

} // namespace NuggetsInc

#endif // HAPTICS_H
