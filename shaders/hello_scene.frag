#version 450
#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_EXT_debug_printf : enable

const float M_PI = 3.1415926538;

struct Material {
    vec4 color;
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

layout(location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo {
    mat4 projection;
    mat4 view;
    mat4 invView;
}
ubo;


layout(set = 0, binding = 1, scalar) uniform Mat { Material[1000] materials; }
m;

layout(set = 0, binding = 2) uniform sampler2D textures[1000];

layout(set = 0, binding = 3) uniform samplerCube samplerCubeMap;

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

//////////////////////////////////////////////////////////////////////////
// Disney BRDF
// based on Acerola implementation (https://github.com/GarrettGunnell/Disney-PBR/blob/main/Assets/Shaders/DisneyBRDF.shader)
// and schuttejoe https://schuttejoe.github.io/post/disneybsdf/
//////////////////////////////////////////////////////////////////////////

float luminance(vec3 color) { return dot(color, vec3(0.299f, 0.587f, 0.114f)); }

float SchlickFresnel(float x) {
    x = clamp((1.0f - x), 0.0, 1.0);
    float x2 = x * x;

    return x2 * x2 * x;  // While this is equivalent to pow(1 - x, 5) it is two less mult instructions
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) { return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0); }

float rcp(float x) { return 1.0f / x; }

float GTR1(float ndoth, float a) {
    float a2 = a * a;
    float t = 1.0f + (a2 - 1.0f) * ndoth * ndoth;
    return (a2 - 1.0f) / (M_PI * log(a2) * t);
}

float DistributionGGX(float ndoth, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH2 = ndoth * ndoth;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = M_PI * denom * denom;

    return num / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}
float GeometrySmith(float ndotl, float ndotv, float roughness) {
    float ggx2 = GeometrySchlickGGX(ndotv, roughness);
    float ggx1 = GeometrySchlickGGX(ndotl, roughness);
    return ggx1 * ggx2;
}

struct BRDFResults {
    vec3 diffuse;
    vec3 specular;
    vec3 clearcoat;
};

BRDFResults DisneyBRDF(vec3 baseColor, float metallic, float roughness, vec3 N, vec3 V, vec3 L) {
    BRDFResults result;
    result.diffuse = vec3(0.0f);
    result.specular = vec3(0.0f);
    result.clearcoat = vec3(0.0f);

    vec3 H = normalize(L + V);

    float ndotl = dotClamp(N, L);
    float ndotv = dotClamp(N, V);
    float ndoth = dotClamp(N, H);
    float ldoth = dotClamp(L, H);

    float Cdlum = luminance(baseColor);
    float _Specular = 0.5f;
    vec3 Ctint = (Cdlum > 0.0f) ? (baseColor / Cdlum) : vec3(1.0f);
    vec3 Cspec0 = mix(_Specular * 0.08f * mix(vec3(1.0f), Ctint, 1.0), baseColor, metallic);
    //         float3 Csheen = lerp(1.0f, Ctint, _SheenTint);

    // Disney Diffuse
    float FL = SchlickFresnel(ndotl);
    float FV = SchlickFresnel(ndotv);

    float Fss90 = ldoth * ldoth * roughness;
    float Fd90 = 0.5f + 2.0f * Fss90;

    float Fd = mix(1.0f, Fd90, FL) * mix(1.0f, Fd90, FV);

    // Subsurface Diffuse (Hanrahan-Krueger brdf approximation)

    float Fss = mix(1.0f, Fss90, FL) * mix(1.0f, Fss90, FV);
    float ss = 1.25f * (Fss * (rcp(ndotl + ndotv) - 0.5f) + 0.5f);

    // Specular
    float NDF = DistributionGGX(ndoth, roughness);
    float G = GeometrySmith(ndotl, ndotv, roughness);

    float FH = SchlickFresnel(ldoth);
    vec3 F = mix(Cspec0, vec3(1.0f), FH);

    result.diffuse = (1.0f / M_PI) * (mix(Fd, ss, 0) * baseColor + 0) * (1 - metallic);
    result.specular = F * G * NDF / (4 * ndotl * ndotv);
    return result;
}

vec3 LearnOpenGLBRDF(vec3 baseColor, float metallic, float roughness, vec3 N, vec3 V, vec3 L, vec3 radiance) {
    BRDFResults result;
    result.diffuse = vec3(0.0f);
    result.specular = vec3(0.0f);
    result.clearcoat = vec3(0.0f);

    vec3 H = normalize(L + V);

    float ndoth = dotClamp(N, H);
    float ndotl = dotClamp(N, L);
    float ndotv = dotClamp(N, V);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, baseColor, metallic);
    vec3 F = fresnelSchlick(max(ndotv, 0.0), F0);

    float NDF = DistributionGGX(ndoth, roughness);
    float G = GeometrySmith(ndotl, ndotv, roughness);
    vec3 numerator = NDF * G * F;

    float denominator = 4.0 * ndotv * ndotl  + 0.0001;
    vec3 specular     = numerator / denominator;  

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;

    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);        


    return  (kD * baseColor / M_PI + specular) * radiance * ndotl;

}

//////////////////////////////////////////////////////////////////////////
// hemisphere sampling
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Main
//////////////////////////////////////////////////////////////////////////

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
    // int texId = m.materials[fragmaterial].albedo_tex_id;
    // // vec3 H = normalize(sun + view);
    // debugPrintfEXT("%i \n", fragmaterial.albedo_tex_id);


    if (fragmaterial.albedo_tex_id != -1) {

        textColor = texture(textures[fragmaterial.albedo_tex_id], fraguv);
    } else {
        textColor = fragmaterial.color;
    }
    vec3 surfaceNormal = normalize(fragNormalWorld);
    if (!gl_FrontFacing) {
        surfaceNormal = -surfaceNormal;
    }

    if (fragmaterial.normal_tex_id != -1) {
        surfaceNormal = perturb_normal(surfaceNormal, view, fragmaterial.normal_tex_id, fraguv);
    }

    if (fragmaterial.metallic_roughness_tex_id != -1) {
        metalRoughness = textureLod(textures[fragmaterial.metallic_roughness_tex_id], fraguv, 0).rg;
    } else {
        metalRoughness = vec2(fragmaterial.metallic, fragmaterial.roughness);
    }

    if (textColor.a < 0.3) {
        discard;
    }

    // // vec3 sunDirection = vec3(0.744015, 0.666869, 0.0415573);
    vec3 color = vec3(0.0);

    // metalRoughness.g = max(0.01, metalRoughness.g); ;

    vec3 diffuse_lightDir = surfaceNormal;
    vec3 reflect_lightDir = reflect(-view, surfaceNormal);

    vec4 diffuse_cubeMapColor = textureLod(samplerCubeMap, diffuse_lightDir, pow(textureQueryLevels(samplerCubeMap), metalRoughness.g));
    vec4 reflect_cubeMapColor = textureLod(samplerCubeMap, reflect_lightDir, pow(textureQueryLevels(samplerCubeMap), metalRoughness.g));
    // BRDFResults res = DisneyBRDF(textColor.rgb, metalRoughness.r, metalRoughness.g, surfaceNormal, view, lightDir);
    // color += ((res.diffuse + res.specular) * cubeMapColor.rgb);

    vec3 color_difuse = LearnOpenGLBRDF(textColor.rgb, metalRoughness.r, metalRoughness.g, surfaceNormal, view, diffuse_lightDir, diffuse_cubeMapColor.rgb );
    vec3 color_reflect = LearnOpenGLBRDF(textColor.rgb, metalRoughness.r, metalRoughness.g, surfaceNormal, view, reflect_lightDir, reflect_cubeMapColor.rgb );
    
    color +=  vec3(0.03) * textColor.rgb;
    color += color_difuse;//mix(color_reflect,color_difuse , metalRoughness.r);

    outColor = vec4(color, 1); 
}