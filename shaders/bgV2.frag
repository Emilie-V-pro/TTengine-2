#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require


const float M_PI = 3.1415926538;

layout(location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;

struct Camera_data {
    mat4 projection;
    mat4 view;
    mat4 invView;
};

struct Material {
    vec3 color;
    float metallic;

    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
};

struct Object_data {
    mat4 world_matrix;
    mat4 normal_matrix;
    uint material_offset;
    vec3 padding;
};

layout(buffer_reference, std430) readonly buffer ObjectBuffer { Object_data data[]; };
layout(buffer_reference, std430) readonly buffer MaterialBuffer { Material data[]; };
layout(buffer_reference, std430) readonly buffer CameraBuffer { Camera_data data[]; };

layout(set = 0, binding = 0) uniform sampler2D textures[1000];

layout(set = 0, binding = 1) uniform samplerCube samplerCubeMap;

layout(push_constant) uniform constants {
    ObjectBuffer objBuffer;
    MaterialBuffer matBuffer;
    CameraBuffer camBuffer;
    uint camera_id;
}pc;

void main() {
    outColor = texture(samplerCubeMap, inUVW);
    // outColor = vec4(inUVW, 1.0);
}
