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

layout(location = 0) in vec3 fragPosWorld;
layout(location = 1) in vec3 fragNormalWorld;
layout(location = 2) in vec2 fraguv;
layout(location = 3) in flat Material fragmaterial;

layout(location = 0) out vec4 albedo_mettalic;
layout(location = 1) out vec4 normal_roughness;

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
    LightBuffer lightBuffer;
    uint camera_id;
    uint nbLight;
}
pc;

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
    vec3 map = texture(textures[texId], fraguv).xyz;
    map.y = 1. - map.y; // Invert Y for normal maps
    map = map * 255. / 127. - 128. / 127.;
    mat3 TBN = cotangent_frame(N, -V, texcoord);
    return normalize(TBN * map);
}

float dotClamp(vec3 v1, vec3 v2) { return clamp(dot(v1, v2), 0.0, 1.0); }

//////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////

void main() {

    vec4 textColor;
    vec2 metalRoughness;
    vec3 pos = pc.camBuffer.data[pc.camera_id].invView[3].xyz;
    vec3 view = normalize(pos - fragPosWorld);


    if (nonuniformEXT(fragmaterial.albedo_tex_id) != -1) {
        textColor = texture(textures[nonuniformEXT(fragmaterial.albedo_tex_id)], fraguv) * vec4(fragmaterial.color, 1);
    } else {
        textColor = vec4(fragmaterial.color, 1);
    }

    vec3 surfaceNormal = normalize(fragNormalWorld);
    if (!gl_FrontFacing) {
        surfaceNormal = -surfaceNormal;
    }

    if (nonuniformEXT(fragmaterial.normal_tex_id) != -1) {
        surfaceNormal = perturb_normal(surfaceNormal, view, nonuniformEXT(fragmaterial.normal_tex_id), fraguv);
    }

    if (nonuniformEXT(fragmaterial.metallic_roughness_tex_id) != -1) {
        metalRoughness = texture(textures[nonuniformEXT(fragmaterial.metallic_roughness_tex_id)], fraguv).bg *
                         vec2(fragmaterial.metallic, fragmaterial.roughness);
    } else {
        metalRoughness = vec2(fragmaterial.metallic, fragmaterial.roughness);
    }


    if (textColor.a < 0.3) {
        discard;
    }


    albedo_mettalic = vec4(textColor.rgb, metalRoughness.r);
    normal_roughness = vec4(surfaceNormal, metalRoughness.g);

}