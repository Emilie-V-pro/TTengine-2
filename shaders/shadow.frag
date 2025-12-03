#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_scalar_block_layout : require
const float M_PI = 3.1415926538;

struct Material {
    vec3 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
};

layout(location = 0) in vec2 fraguv;
layout(location = 1) in flat int alphaTextureID;


struct Camera_data {
    mat4 projection;
    mat4 view;
    mat4 invView;
};

struct Object_data {
    mat4 world_matrix;
    mat4 normal_matrix;
    vec3 padding;
    uint material_offset;
};

struct Light{
    vec4 color;
    vec3 pos;
    uint Type;
    vec3 orienation;
    uint offset;
};

layout(buffer_reference, std430) readonly buffer ObjectBuffer { Object_data data[]; };
layout(buffer_reference, std430) readonly buffer MaterialBuffer { Material data[]; };
layout(buffer_reference, std430) readonly buffer CameraBuffer { Camera_data data[]; };
layout(buffer_reference, std430) readonly buffer LightBuffer { Light data[]; };

layout(set = 0, binding = 0) uniform sampler2D textures[1000];

layout(set = 0, binding = 1) uniform samplerCube samplerCubeMap;

layout(push_constant) uniform constants {
    ObjectBuffer objBuffer;
    MaterialBuffer matBuffer;
    CameraBuffer camBuffer;
    uint camera_id;
}pc;



//////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////

void main() {

    vec4 textColor;
    vec2 metalRoughness;
    vec3 pos = pc.camBuffer.data[pc.camera_id].invView[3].xyz;


    if (nonuniformEXT(alphaTextureID) != -1) {
        textColor = texture(textures[nonuniformEXT(alphaTextureID)], fraguv);
    } else {
        textColor = vec4(1);
    }

    if (textColor.a < 0.3) {
        discard;
    }
}