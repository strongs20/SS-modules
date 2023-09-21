// scale.cpp

#include "scale.hpp"

namespace ScaleUtils
{

    const int NUM_SCALES = 13; // Update this as you add more scales

    int SCALE_MAJOR[] = {0, 2, 4, 5, 7, 9, 11, 12};
    int SCALE_LYDIAN[] = {0, 2, 4, 6, 7, 9, 11, 12};
    int SCALE_MIXOLYDIAN[] = {0, 2, 4, 5, 7, 9, 10, 12};
    int SCALE_MINOR[] = {0, 2, 3, 5, 7, 8, 10, 12};
    int SCALE_MELODIC_MINOR[] = {0, 2, 3, 5, 7, 9, 11, 12};
    int SCALE_HARMONIC_MINOR[] = {0, 2, 3, 5, 7, 8, 11, 12};
    int SCALE_DORIAN[] = {0, 2, 3, 5, 7, 9, 10, 12};
    int SCALE_PHRYGIAN[] = {0, 1, 3, 5, 7, 8, 10, 12};
    int SCALE_AEOLIAN[] = {0, 2, 3, 5, 7, 8, 10, 12};
    int SCALE_CHROMATIC[] = {0, 1, 2, 3, 4, 5, 6, 7,
                             8, 9, 10, 11, 12};
    int SCALE_RAGA_KAFI[] = {0, 1, 3, 5, 7, 8, 10, 12};
    int SCALE_MAQAM_HIJAZ[] = {0, 1, 4, 5, 7, 8, 11, 12};
    int SCALE_JAPANESE_KUMOI[] = {0, 2, 3, 7, 9, 12};

    int *SCALE_ARRAY[] = {
        SCALE_MAJOR,
        SCALE_LYDIAN,
        SCALE_MIXOLYDIAN,
        SCALE_MINOR,
        SCALE_MELODIC_MINOR,
        SCALE_HARMONIC_MINOR,
        SCALE_DORIAN,
        SCALE_PHRYGIAN,
        SCALE_AEOLIAN,
        SCALE_CHROMATIC,
        SCALE_RAGA_KAFI,
        SCALE_MAQAM_HIJAZ,
        SCALE_JAPANESE_KUMOI,
    };

    int SCALE_SIZE[] = {
        8,
        8,
        8,
        8,
        8,
        8,
        8,
        8,
        8,
        13,
        8,
        8,
        6,
    };

    std::string SCALE_NAMES[] = {
        "Major",
        "Lydian",
        "Mixolydian",
        "Minor",
        "Melodic Minor",
        "Harmonic Minor",
        "Dorian",
        "Phrygian",
        "Aeolian",
        "Chromatic",
        "Raga Kafi",
        "Maqam Hijaz",
        "Japanese Kumoi"};

    std::string KEY_NAMES[] = {"C", "Db", "D", "Eb", "E", "F",
                               "Gb", "G", "Ab", "A", "Bb", "B"};
}
