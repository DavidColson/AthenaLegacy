#pragma once

float LinearMap(float x, float fromMin, float fromMax, float toMin, float toMax)
{
    return toMin + ((x - fromMin) / (fromMax - fromMin)) * (toMax - toMin);
}