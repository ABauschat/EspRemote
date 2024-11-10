#ifndef SOUNDS_H
#define SOUNDS_H

#include <Arduino.h>

namespace NuggetsInc {

class Sounds {
public:
    static Sounds& getInstance();

    // Prevent copying
    Sounds(const Sounds&) = delete;
    Sounds& operator=(const Sounds&) = delete;

    // Frequencies for notes (in Hz)
    enum Notes {
        NOTE_E4  = 330,
        NOTE_G4  = 392,
        NOTE_AS4 = 466,
        NOTE_B4  = 494,
        NOTE_C5  = 523,
        NOTE_D5  = 587,
        NOTE_E5  = 659,
        NOTE_F5  = 698,
        NOTE_G5  = 784
    };

    // Sound functions
    void playMelody();
    void playTone(uint32_t frequency, uint32_t duration);

private:
    Sounds(); // Private constructor
};

} // namespace NuggetsInc

#endif // SOUNDS_H
