// scale.hpp
#include <string>

#pragma once

namespace ScaleUtils
{

    extern const int NUM_SCALES;

    enum ScaleEnum
    {
        MAJOR,
        LYDIAN,
        MIXOLYDIAN,
        MINOR,
        MELODIC_MINOR,
        HARMONIC_MINOR,
        DORIAN,
        PHRYGIAN,
        AEOLIAN,
        CHROMATIC,
        RAGA_KAFI,
        MAQAM_HIJAZ,
        JAPANESE_KUMOI,
        NONE
    };

    extern int *SCALE_ARRAY[];
    extern int SCALE_SIZE[];
    extern std::string SCALE_NAMES[];
    extern std::string KEY_NAMES[];
}
