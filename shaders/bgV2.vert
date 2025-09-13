#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
#extension GL_EXT_debug_printf : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint material;

layout(location = 0) out vec3 outUVW;

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
    LightBuffer lightBuffer;
    uint camera_id;
    uint nbLight;
}pc;


void main() {
    outUVW = normalize(position * 10);
    Camera_data c = pc.camBuffer.data[pc.camera_id];
    mat4 viewMat = mat4(mat3(c.view));
    gl_Position = c.projection * viewMat * vec4(position.xyz, 1.0);
    // if(gl_VertexIndex == 0){
        // debugPrintfEXT("pos : %f %f %f | normal : %f %f %f | uv : %f %f | mat %i", position.x, position.y, position.z, normal.x, normal.y, normal.z, uv.x, uv.y, material);
    // }
    debugPrintfEXT("re");
}
