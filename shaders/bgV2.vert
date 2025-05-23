#version 450


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 uv;
layout(location = 3) in uint material;

layout (location = 0) out vec3 outUVW;


struct Camera_data {
     mat4 projection;
    mat4 view;
    mat4 invView;
};

layout(set = 0, binding = 0) uniform UBO {
    Camera_data cameras[20];
}ubo;


struct Material {
    vec3 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
};
layout(set = 0, binding = 1) uniform Mat { Material[1000] materials; }
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];

layout(set = 0 , binding = 3) uniform samplerCube samplerCubeMap;

layout(push_constant) uniform constants {
    uint camera_id;
}
pc;


void main() {
    outUVW = normalize(position * 10) ;
	// Convert cubemap coordinates into Vulkan coordinate space
	// outUVW.xy *= -1.0;
	// Remove translation from view matrix
    Camera_data c = ubo.cameras[pc.camera_id];
	mat4 viewMat = mat4(mat3(c.view));
	gl_Position = c.projection * viewMat * vec4(position.xyz, 1.0);
}
