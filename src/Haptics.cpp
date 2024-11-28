#include "Haptics.h"
#include "Device.h"

namespace NuggetsInc {

Haptics& Haptics::getInstance() {
    static Haptics instance;
    return instance;
}

Haptics::Haptics()
    : hapticsTaskHandle(nullptr), hapticsQueue(nullptr) {
    // Constructor code if needed
}

void Haptics::begin() {
    if (hapticsTaskHandle == nullptr) {
        // Create the haptics queue
        hapticsQueue = xQueueCreate(10, sizeof(VibrationCommand));
        if (hapticsQueue == nullptr) {
            // Handle error
            Serial.println("Failed to create haptics queue");
            return;
        }

        // Create the haptics task pinned to core 0
        xTaskCreatePinnedToCore(
            hapticsTask,         // Task function
            "HapticsTask",       // Name of the task
            2048,                // Stack size (in words)
            this,                // Task input parameter
            1,                   // Priority of the task
            &hapticsTaskHandle,  // Task handle
            0                    // Core to run the task on (0 or 1)
        );
    }
}

void Haptics::end() {
    if (hapticsTaskHandle != nullptr) {
        vTaskDelete(hapticsTaskHandle);
        hapticsTaskHandle = nullptr;
    }
    if (hapticsQueue != nullptr) {
        vQueueDelete(hapticsQueue);
        hapticsQueue = nullptr;
    }
}

void Haptics::singleVibration() {
    VibrationCommand cmd = { SINGLE };
    if (hapticsQueue != nullptr) {
        xQueueSend(hapticsQueue, &cmd, portMAX_DELAY);
    }
}

void Haptics::doubleVibration() {
    VibrationCommand cmd = { DOUBLE };
    if (hapticsQueue != nullptr) {
        xQueueSend(hapticsQueue, &cmd, portMAX_DELAY);
    }
}

void Haptics::hapticsTask(void* parameter) {
    Haptics* haptics = static_cast<Haptics*>(parameter);

    VibrationPattern currentPattern = NONE;
    unsigned long patternStartTime = 0;
    unsigned long vibrationDurations[4];
    uint8_t vibrationStep = 0;

    while (true) {
        VibrationCommand cmd;
        // Check if a new command is received
        if (xQueueReceive(haptics->hapticsQueue, &cmd, 0)) {
            // New command received
            currentPattern = cmd.pattern;
            patternStartTime = 0;
            vibrationStep = 0;

            if (currentPattern == SINGLE) {
                // Define the pattern: Single vibration of 200ms
                vibrationDurations[0] = 250; // Vibration duration
                vibrationDurations[1] = 0;   // End of pattern
            } else if (currentPattern == DOUBLE) {
                // Define the pattern: Two vibrations of 200ms with 100ms pause
                vibrationDurations[0] = 250; // Vibration on
                vibrationDurations[1] = 100; // Pause
                vibrationDurations[2] = 250; // Vibration on
                vibrationDurations[3] = 0;   // End of pattern
            }
        }

        if (currentPattern != NONE) {
            unsigned long currentTime = millis();

            if (patternStartTime == 0) {
                // Pattern just started
                patternStartTime = currentTime;
                Device::getInstance().startVibration();
            }

            if (currentTime - patternStartTime >= vibrationDurations[vibrationStep]) {
                patternStartTime = currentTime;
                vibrationStep++;

                if (vibrationDurations[vibrationStep] == 0) {
                    // End of pattern
                    Device::getInstance().stopVibration();
                    currentPattern = NONE;
                    vibrationStep = 0;
                    patternStartTime = 0;
                } else {
                    if (vibrationStep % 2 == 0) {
                        // Even step: vibration on
                        Device::getInstance().startVibration();
                    } else {
                        // Odd step: vibration off
                        Device::getInstance().stopVibration();
                    }
                }
            }
        } else {
            // Ensure vibration is off when there's no pattern
            Device::getInstance().stopVibration();
        }
        // Yield to other tasks
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}

} // namespace NuggetsInc
