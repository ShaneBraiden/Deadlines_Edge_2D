#pragma once

// General-purpose math utilities as function templates.
//
// Lab requirements covered:
//   - Function templates: clamp<T>, lerp<T>

namespace Utility {

    // Constrains a value to the range [min, max].
    // Lab: function template
    template<typename T>
    T clamp(T value, T min, T max) {
        if (value < min) return min;
        if (value > max) return max;
        return value;
    }

    // Linear interpolation from a to b by factor t (0 = a, 1 = b).
    // Lab: function template
    template<typename T>
    T lerp(T a, T b, T t) {
        return a + (b - a) * t;
    }

}
