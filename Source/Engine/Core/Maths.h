#pragma once

inline float LinearMap(float x, float fromMin, float fromMax, float toMin, float toMax)
{
    return toMin + ((x - fromMin) / (fromMax - fromMin)) * (toMax - toMin);
}

inline int mod_floor(int a, int n) {
    return ((a % n) + n) % n;
}

inline int mod_floor(int a, size_t n) {
    return mod_floor(a, (int)n);
}