#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const float M_PI = 3.1415926538;

layout (location = 0) in vec3 inUVW;

layout(location = 0) out vec4 outColor;



layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
}
ubo;


struct Material {
    vec4 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
};
layout(set = 0, binding = 1, scalar) uniform Mat { Material[1000] materials; }
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];

layout(set = 0 , binding = 3) uniform samplerCube samplerCubeMap;



void main() {


    
    // outColor = texture(samplerCubeMap, inUVW);
    outColor = vec4(inUVW, 1.0);

}