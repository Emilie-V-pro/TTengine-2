#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_buffer_reference2 : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint material;


struct Material {
    vec3 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
};

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fraguv;
layout(location = 3) out flat Material fragmaterial;

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
    vec4 positionWorld = pc.objBuffer.data[gl_InstanceIndex].world_matrix * vec4(position, 1.0);
    Camera_data c = pc.camBuffer.data[pc.camera_id];
    gl_Position = c.projection * c.view * positionWorld;
    fragNormalWorld = normalize(mat3(pc.objBuffer.data[gl_InstanceIndex].normal_matrix) * normal);
    
    fragPosWorld =  positionWorld.xyz ;
    fragmaterial = pc.matBuffer.data[material];
    fraguv = uv;
}
