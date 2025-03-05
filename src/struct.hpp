#pragma once

#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <string>

namespace TTe {

struct Material {
    std::string name;
    glm::vec4 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;

};

struct MaterialGPU {
    glm::vec4 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
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
}  // namespace TTe