#pragma once
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <glm/ext/scalar_common.hpp>
#include <glm/geometric.hpp>
#include <glm/trigonometric.hpp>

#include <glm/ext/matrix_transform.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/transform.hpp>

namespace TTe {
inline glm::quat eulerZXYtoQuat(glm::vec3 euler) {
    // Convertir les angles d'Euler en quaternion
    float c1 = cos(euler.x / 2);
    float c2 = cos(euler.y / 2);
    float c3 = cos(euler.z / 2);

    float s1 = sin(euler.x / 2);
    float s2 = sin(euler.y / 2);
    float s3 = sin(euler.z / 2);

    glm::quat q = {c1 * c2 * c3 - s1 * s2 * s3, s1 * c2 * c3 - c1 * s2 * s3, c1 * s2 * c3 + s1 * c2 * s3, c1 * c2 * s3 + s1 * s2 * c3};

    return q;  // Ordre de multiplication ZXY
}

inline glm::quat eulerZYXtoQuat(glm::vec3 euler) {
    // Convertir les angles d'Euler en quaternion
    float c1 = cos(euler.x / 2);
    float c2 = cos(euler.y / 2);
    float c3 = cos(euler.z / 2);

    float s1 = sin(euler.x / 2);
    float s2 = sin(euler.y / 2);
    float s3 = sin(euler.z / 2);

    glm::quat q = {c1 * c2 * c3 + s1 * s2 * s3, s1 * c2 * c3 - c1 * s2 * s3, c1 * s2 * c3 + s1 * c2 * s3, c1 * c2 * s3 - s1 * s2 * c3};

    return q;  // Ordre de multiplication ZXY
}

inline glm::vec3 threeaxisrot(double r11, double r12, double r21, double r31, double r32) {
    return glm::vec3(asin(r21), atan2(r31, r32), atan2(r11, r12));
}

inline glm::vec3 threeaxisrot2(double r11, double r12, double r21, double r31, double r32) {
    return glm::vec3(atan2(r31, r32), asin(r21), atan2(r11, r12));
}

inline glm::vec3 quatToEulerZXY(glm::quat q) {
    return threeaxisrot(
        -2 * (q.x * q.y - q.w * q.z), q.w * q.w - q.x * q.x + q.y * q.y - q.z * q.z, 2 * (q.y * q.z + q.w * q.x),
        -2 * (q.x * q.z - q.w * q.y), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);
}

inline glm::vec3 quatToEulerZYX(glm::quat q) {
    return threeaxisrot2(
        2 * (q.x * q.y + q.w * q.z), q.w * q.w + q.x * q.x - q.y * q.y - q.z * q.z, -2 * (q.x * q.z - q.w * q.y),
        2 * (q.y * q.z + q.w * q.x), q.w * q.w - q.x * q.x - q.y * q.y + q.z * q.z);
}
}