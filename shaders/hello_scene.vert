#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint material;

layout(location = 0) out vec3 fragPosWorld;
layout(location = 1) out vec3 fragNormalWorld;
layout(location = 2) out vec2 fraguv;
layout(location = 3) out flat uint fragmaterial;

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

layout(set = 0, binding = 1, scalar) uniform Mat { Material[1000] materials; }
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];

layout(set = 0 , binding = 3) uniform samplerCube samplerCubeMap;

struct ObjectInfo {
    mat4 modelMatrix;
    mat4 normalMatrix;
};

// layout(buffer_reference, scalar) readonly buffer InstanceBuffer2 { ObjectInfo objInfo[]; };
layout(push_constant) uniform constants {
    mat4 modelMatrix;
    mat4 normalMatrix;
}
PushConstants;

void main() {
    vec4 positionWorld = PushConstants.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;
    fragNormalWorld = normalize(mat3(PushConstants.normalMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragmaterial = material;
    fraguv = uv;
}
