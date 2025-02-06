#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require

const float M_PI = 3.1415926538;

layout(location = 0) in vec2 fragOffset;

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

layout(set = 0, binding = 1) uniform Mat { Material[1000] materials; }
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];

vec3 createRay(in ivec2 px) {
    // convert pixel to NDS
    vec2 pxNDS = ((vec2(px.x, 720 - px.y)) / vec2(1280, 720)) * 2. - 1.;

    // choose an arbitrary HitPoint in the viewing volume
    // z = -1 equals a HitPoint on the near plane, i.e. the screen
    vec3 HitPointNDS = vec3(pxNDS, 0.1);

    // as this is in homogenous space, add the last homogenous coordinate
    vec4 HitPointNDSH = vec4(HitPointNDS, 1.0);
    // transform by inverse projection to get the HitPoint in view space
    vec4 dirEye = inverse(ubo.projection) * HitPointNDSH;

    // since the camera is at the origin in view space by definition,
    // the current HitPoint is already the correct direction
    // (dir(0,P) = P - 0 = P as a direction, an infinite HitPoint,
    // the homogenous component becomes 0 the scaling done by the
    // w-division is not of interest, as the direction in xyz will
    // stay the same and we can just normalize it later
    dirEye.w = 0.;

    // compute world ray direction by multiplying the inverse view matrix
    vec3 rd = normalize((ubo.invView * dirEye).xyz);
    return rd;
}

void main() {
    ivec2 pixelPos = ivec2(gl_FragCoord.xy);
    vec3 ray = createRay(pixelPos);
    outColor = vec4(ray, 1);


    // color = PBR(sun, view, surfaceNormal, vec3(1, 1, 1), metalRoughness.r, metalRoughness.g, textColor.rgb, 2) + textColor.rgb * ambiant;
    // color = color / (color + vec3(1.0));
    // color = pow(color, vec3(1.6f / 2.2));
}