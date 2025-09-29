#pragma once

#include <algorithm>
#include <functional>
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>

namespace TTe {

struct Material {
    std::string name;
    glm::vec3 color = glm::vec3(1, 1, 1);
    float metallic = 0;
    float roughness = 0.9;
    int albedo_tex_id = -1;
    int metallic_roughness_tex_id = -1;
    int normal_tex_id = -1;

    void applyTextureOffset(uint offset) {
        albedo_tex_id += (albedo_tex_id != -1) ? offset : 0;
        metallic_roughness_tex_id += (metallic_roughness_tex_id != -1) ? offset : 0;
        normal_tex_id += (normal_tex_id != -1) ? offset : 0;
    }
};

struct MaterialGPU {
    glm::vec3 color;
    float metallic;
    float roughness;
    int albedo_tex_id = -1;
    int metallic_roughness_tex_id = -1;
    int normal_tex_id = -1;
};

struct Vertex {
    glm::vec3 pos;
    glm::vec3 normal;
    glm::vec2 uv;
    uint32_t material_id;
};

struct Ubo {
    glm::mat4 projection;
    glm::mat4 view;
    glm::mat4 invView;
};

struct BoundingBox {
    glm::vec3 pmin = {0,0,0};
    glm::vec3 pmax = {0,0,0};

    float intersect(glm::vec3 &origin, glm::vec3 &direction) {
        float txpmin = (pmin.x - origin.x) / direction.x;
        float txpmax = (pmax.x - origin.x) / direction.x;

        if (txpmin > txpmax) std::swap(txpmin, txpmax);

        float typmin = (pmin.y - origin.y) / direction.y;
        float typmax = (pmax.y - origin.y) / direction.y;
        if (typmin > typmax) std::swap(typmin, typmax);

        float tzpmin = (pmin.z - origin.z) / direction.z;
        float tzpmax = (pmax.z - origin.z) / direction.z;

        if (tzpmin > tzpmax) std::swap(tzpmin, tzpmax);
     

        float t_min = std::max({txpmin, typmin, tzpmin});
        float t_max = std::min({txpmax, typmax, tzpmax});

        float t_final = (t_min < t_max) ?  t_min : -1;

        return t_final;
    }
};

struct SceneHit{
    glm::vec3 normal;
    float t;
    uint32_t node_index;
    uint32_t material_id;
};

class Watchedvec3 {
   public:
    glm::dvec3 value;
    std::function<void()> onChanged;

    Watchedvec3() = default;
    Watchedvec3(const glm::dvec3& v) : value(v) {}

    Watchedvec3& operator=(const glm::dvec3& v) {
        value = v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3(const glm::dvec3&& v) : value(v) {}

    Watchedvec3& operator=(const glm::dvec3&& v) {
        value = v;
        if (onChanged) onChanged();
        return *this;
    }

    // access to value with .
    glm::dvec3* operator->() { return &value; }
    const glm::dvec3* operator->() const { return &value; }

    operator glm::dvec3&() { return value; }
    operator const glm::dvec3&() const { return value; }

    Watchedvec3& operator+=(const glm::dvec3& v) {
        value += v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3& operator-=(const glm::dvec3& v) {
        value -= v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3& operator*=(const glm::dvec3& v) {
        value *= v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3& operator/=(const glm::dvec3& v) {
        value /= v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3& operator+=(float v) {
        value += v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3& operator-=(float v) {
        value -= v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3& operator*=(float v) {
        value *= v;
        if (onChanged) onChanged();
        return *this;
    }

    Watchedvec3& operator/=(float v) {
        value /= v;
        if (onChanged) onChanged();
        return *this;
    }

    friend glm::dvec3 operator+(Watchedvec3 lhs, const glm::dvec3& rhs) { return lhs.value + rhs; }

    friend glm::dvec3 operator-(Watchedvec3 lhs, const glm::dvec3& rhs) { return lhs.value - rhs; }

    friend glm::dvec3 operator*(Watchedvec3 lhs, const glm::dvec3& rhs) { return lhs.value * rhs; }

    friend glm::dvec3 operator/(Watchedvec3 lhs, const glm::dvec3& rhs) { return lhs.value / rhs; }

    friend glm::dvec3 operator+(Watchedvec3 lhs, double rhs) { return lhs.value + rhs; }

    friend glm::dvec3 operator-(Watchedvec3 lhs, double rhs) { return lhs.value - rhs; }

    friend glm::dvec3 operator*(Watchedvec3 lhs, double rhs) { return lhs.value * rhs; }

    friend glm::dvec3 operator/(Watchedvec3 lhs, double rhs) { return lhs.value / rhs; }

    friend glm::dvec3 operator+(double lhs, Watchedvec3 rhs) { return lhs + rhs.value; }

    friend glm::dvec3 operator-(double lhs, Watchedvec3 rhs) { return lhs - rhs.value; }

    friend glm::dvec3 operator*(double lhs, Watchedvec3 rhs) { return lhs * rhs.value; }

    friend glm::dvec3 operator/(double lhs, Watchedvec3 rhs) { return lhs / rhs.value; }

    friend bool operator==(const Watchedvec3& lhs, const glm::dvec3& rhs) { return lhs.value == rhs; }
};

struct TransformComponent {
    Watchedvec3 pos{glm::dvec3(0.0f)};
    Watchedvec3 rot{glm::dvec3(0.0f)};
    Watchedvec3 scale{glm::dvec3(1.0f)};
};

}  // namespace TTe