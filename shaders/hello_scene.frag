#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const float M_PI = 3.1415926538;

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fraguv;
layout(location = 3) in flat uint fragmaterial;

layout(location = 0) out vec4 outColor;

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


// http://www.thetenthplanet.de/archives/1180
mat3 cotangent_frame(vec3 N, vec3 p, vec2 uv) {
    vec3 dp1 = dFdx(p);
    vec3 dp2 = dFdy(p);
    vec2 duv1 = dFdx(uv);
    vec2 duv2 = dFdy(uv);

    vec3 dp2perp = cross(dp2, N);
    vec3 dp1perp = cross(N, dp1);
    vec3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    vec3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = inversesqrt(max(dot(T, T), dot(B, B)));
    return mat3(T * invmax, B * invmax, N);
}

vec3 perturb_normal(vec3 N, vec3 V, int texId, vec2 texcoord) {
    // N, la normale interpolée et
    // V, le vecteur vue (vertex dirigé vers l'œil)
    vec3 map = textureLod(textures[texId], fraguv, 0).xyz;
    map = map * 255. / 127. - 128. / 127.;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}


float dotClamp(vec3 v1, vec3 v2) { return clamp(dot(v1, v2), 0.0, 1.0); }

void main() {
    // if (false) {
    //     mat4 test = PushConstants.instances.objInfo[0].modelMatrix;
    // }
    vec4 textColor;
    vec2 metalRoughness;
    // vec3 ambiant = vec3(0.01);
    vec3 pos = ubo.invView[3].xyz;
    vec3 view = normalize(pos - fragPosWorld);
    // vec3 sun = normalize(vec3(-1, 1, -1));
    int texId = m.materials[fragmaterial].color_texture_id;
    // vec3 H = normalize(sun + view);
    if (texId != -1) {
        textColor = textureLod(textures[texId], fraguv, 0);
    } else {
        textColor = m.materials[fragmaterial].color;
    }
    vec3 surfaceNormal = normalize(fragNormalWorld);
    if (m.materials[fragmaterial].normalMap_texture_id != -1) {
        surfaceNormal = perturb_normal(surfaceNormal, view, m.materials[fragmaterial].normalMap_texture_id, fraguv);
    }
   
    if (textColor.a < 0.3) {
        discard;
    }
    outColor = vec4(textColor.rgb, 1);


    // color = PBR(sun, view, surfaceNormal, vec3(1, 1, 1), metalRoughness.r, metalRoughness.g, textColor.rgb, 2) + textColor.rgb * ambiant;
    // color = color / (color + vec3(1.0));
    // color = pow(color, vec3(1.6f / 2.2));
}