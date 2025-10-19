#include <cmath>

inline float yFOV_to_FOV(float y_FOV_deg, float aspect_ratio) {
    float y_FOV_rad = y_FOV_deg * M_PI / 180.0f;
    float x_FOV_rad = 2.0f * atan(tan(y_FOV_rad / 2.0f) * aspect_ratio);
    return  x_FOV_rad * 180.0f / M_PI;
}