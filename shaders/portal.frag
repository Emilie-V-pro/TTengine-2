#version 450
#extension GL_EXT_nonuniform_qualifier : require
const float M_PI = 3.1415926538;

struct Material {
    vec3 color;
    float metallic;
    float roughness;
    int albedo_tex_id;
    int metallic_roughness_tex_id;
    int normal_tex_id;
};

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fraguv;
layout(location = 3) in flat uint fragmaterial;

layout(location = 0) out vec4 outColor;

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


layout(set = 1, binding = 0) uniform sampler2D portalTextures[10];


layout(push_constant) uniform constants {
    mat4 modelMatrix;
    mat4 normalMatrix;
    uint camera_id;
    vec3 portal_color;
    uint portal_id;
    uint recurs_id;
}
pc;


void main() {
    vec4 color = vec4(0.0);
    if (fragmaterial != -1) {
        color = vec4(pc.portal_color, 1.0);
    } else {
        // screen space coordinates
        vec2 uv = gl_FragCoord.xy;;
        if (pc.recurs_id == 5) {
            //sample cube map
            vec3 dir = normalize(fragPosWorld - ubo.cameras[pc.camera_id].view[3].xyz);
            color = vec4(textureLod(samplerCubeMap, dir, 6.0).rgb, 1.0);

        } else if (pc.recurs_id == 0) {
            uint portal_id = pc.portal_id;

            do {
                color = texture(portalTextures[portal_id], uv);
                // get new portal id from the alpha channel
                uint next_portal_id = uint(color.a * 255.0);
            } while (color.a != 1.0);
        } 
        
        else {
            uint next_portal_id = ((pc.portal_id % 2) == 0) ? pc.portal_id + 1 : pc.portal_id - 1;
            color = vec4(0,0,0, float(next_portal_id) / 255.0);
        }
    }
    outColor = color;
}