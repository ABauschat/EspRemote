#include "Sounds.h"
#include "Device.h"

namespace NuggetsInc {

Sounds& Sounds::getInstance() {
    static Sounds instance;
    return instance;
}

Sounds::Sounds() {
    // Constructor code if needed
}

void Sounds::playTone(uint32_t frequency, uint32_t duration) {
    Device::getInstance().playTone(frequency, duration);
}

void Sounds::playMelody() {
    uint32_t melody[] = { NOTE_G4, NOTE_G4, NOTE_G4, NOTE_E4 };
    uint32_t noteDurations[] = { 200, 200, 200, 600 };
    uint16_t numNotes = sizeof(melody) / sizeof(melody[0]);

    for (uint16_t i = 0; i < numNotes; i++) {
        playTone(melody[i], noteDurations[i]);
        delay(50); // Short pause between notes
    }
}

} // namespace NuggetsInc
