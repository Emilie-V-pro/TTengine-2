#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

const vec2 OFFSETS[3] = vec2[](
    vec2(-2000, 2000), vec2(0, -4500.5), vec2(2000, 2000)

);



layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
}
ubo;

layout(set = 0 , binding = 1) uniform samplerCube samplerCubeMap;

layout(push_constant) uniform constants {
    ivec2 resolution;
};

void main() {
    vec2 fragOffset = OFFSETS[gl_VertexIndex];
    gl_Position = vec4(fragOffset, 101, 101);
}
