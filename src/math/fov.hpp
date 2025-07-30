#include <cmath>
#include <iostream>

inline float yFOV_to_FOV(float yFOV_deg, float aspectRatio) {
    float yFOV_rad = yFOV_deg * M_PI / 180.0f;
    float xFOV_rad = 2.0f * atan(tan(yFOV_rad / 2.0f) * aspectRatio);
    return  xFOV_rad * 180.0f / M_PI;
}