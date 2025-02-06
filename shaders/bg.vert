#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

const vec2 OFFSETS[3] = vec2[](
    vec2(-2000, 2000), vec2(0, -4500.5), vec2(2000, 2000)

);

layout(location = 0) out vec2 fragOffset;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
}
ubo;

struct Material {
    vec4 color;
    // float metallic;
    // float roughness;
    int color_texture_id;
    // int metallic_roughness_texture_id;
    int normalMap_texture_id;
};

layout(set = 0, binding = 1) uniform Mat { Material[1000] materials; }
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];


void main() {
    fragOffset = OFFSETS[gl_VertexIndex];
    gl_Position = vec4(fragOffset, 101, 101);
}
