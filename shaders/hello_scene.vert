#version 450

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

layout(set = 0, binding = 0) uniform UBO {
    Camera_data cameras[20];
}ubo;





layout(set = 0, binding = 1) uniform Mat { Material[1000] materials; }
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];

layout(set = 0 , binding = 3) uniform samplerCube samplerCubeMap;

// layout(buffer_reference, scalar) readonly buffer InstanceBuffer2 { ObjectInfo objInfo[]; };
layout(push_constant) uniform constants {
        mat4 modelMatrix;
    mat4 normalMatrix;
    vec3 portal_pos;
    uint camera_id;
    vec3 portal_normal;
}
pc;

void main() {
    vec4 positionWorld = pc.modelMatrix * vec4(position, 1.0);
    Camera_data c = ubo.cameras[pc.camera_id];
    gl_Position = c.projection * c.view * positionWorld;
    fragNormalWorld = normalize(mat3(pc.normalMatrix) * normal);
    
    fragPosWorld = (pc.camera_id == 0) ? positionWorld.xyz :  vec3(positionWorld.x, positionWorld.y , positionWorld.z) ;
    fragmaterial = m.materials[material];
    // debugPrintfEXT("%i \n", fragmaterial.albedo_tex_id);
    fraguv = uv;
}
