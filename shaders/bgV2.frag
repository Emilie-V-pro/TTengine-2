#version 450

const float M_PI = 3.1415926538;

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;

struct Camera_data {
    mat4 projection;
    mat4 view;
    mat4 invView;
};

layout(set = 0, binding = 0) uniform UBO {
    Camera_data cameras[20];
} ubo;

struct Material {
    vec3 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
};
layout(set = 0, binding = 1) uniform Mat {
    Material[1000] materials;
}
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];

layout(set = 0, binding = 3) uniform samplerCube samplerCubeMap;

layout(push_constant) uniform constants {
    uint camera_id;
}
pc;
void main() {
    outColor = texture(samplerCubeMap, inUVW);
    // outColor = vec4(inUVW, 1.0);
}
