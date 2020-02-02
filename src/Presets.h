#ifndef GAME_OF_LIFE_PRESETS_H
#define GAME_OF_LIFE_PRESETS_H

#include <vector>
#include "fields/SimpleMatrix.h"

const auto PRESET_GLIDER = SimpleMatrix<bool>(3, 3, {
        false, true, false,
        false, false, true,
        true, true, true
});

const auto PRESET_RPENTOMINO = SimpleMatrix<bool>(3, 3, {
        false, true, true,
        true, true, false,
        false, true, false
});

const auto PRESET_DIEHARD = SimpleMatrix<bool>(3, 8, {
        false, false, false, false, false, false, true, false,
        true, true, false, false, false, false, false, false,
        false, true, false, false, false, true, true, true
});

const auto PRESET_ACORN = SimpleMatrix<bool>(3, 7, {
        false, true, false, false, false, false, false,
        false, false, false, true, false, false, false,
        true, true, false, false, true, true, true
});

const auto PRESET_EVE = SimpleMatrix<bool>(10, 16, {
        false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false,
        false, false, false, false, false, false, false, false, false, false, false, false, false, false, true, false,
        false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, true,
        false, false, false, false, false, false, false, false, false, true, false, false, false, true, true, false,
        false, false, false, false, false, false, false, true, false, false, false, false, true, true, true, false,
        false, false, false, false, false, false, false, true, false, false, false, true, false, false, false, false,
        false, false, false, false, false, false, false, false, false, true, true, false, false, false, false, false,
        false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false,
        true, true, true, false, false, false, false, false, false, false, false, false, false, false, false, false,
        true, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false,
});

#endif //GAME_OF_LIFE_PRESETS_H
