#version 460
#pragma shader_stage(fragment)

const float PI = 3.14159265359;

layout(constant_id = 0) const uint MAX_LIGHT_COUNT = 8;
layout(constant_id = 1) const uint MAX_INSTANCE_COUNT = 8;

struct LightData {
    vec3 lightPosition;
    vec3 lightColor;
};
layout(binding = 0, set = 0) uniform LightUniform {
    uint lightCount;
    LightData light[MAX_LIGHT_COUNT];
};

struct InstanceData {
    mat4 matrixModel;
    mat3 matrixNormal;
    vec3  albedo;
    float metallic;
    float roughness;
    float ao;
};
layout(binding = 1, set = 0) uniform SenceUniform {
    vec3 cameraPosition;
    mat4 matrixCamera;
    InstanceData instance[MAX_INSTANCE_COUNT];
};

layout(location = 0) smooth in vec3 inNormal;
layout(location = 1) in vec3 inWorldPos;
layout(location = 2) flat in uint passIndex;

layout(location = 0) out vec4 outColor;

float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 N = normalize(inNormal); 
    vec3 V = normalize(cameraPosition - inWorldPos);

    vec3  albedo    = instance[passIndex].albedo;
    float metallic  = instance[passIndex].metallic;
    float roughness = instance[passIndex].roughness;
    float ao        = instance[passIndex].ao;

    vec3 F0 = vec3(0.04);
    F0      = mix(F0, albedo, metallic);

    vec3 Lo = vec3(0.0);
    for(int i = 0; i < lightCount; ++i) {
        vec3 L = light[i].lightPosition - inWorldPos;
        float distance = length(L);
        L = L / distance;
        vec3 H = normalize(V + L);

        float attenuation = 1.0 / (distance * distance);
        vec3 radiance     = light[i].lightColor * attenuation;

        // 计算 BRDF
        // 法线分布函数
        float NDF = DistributionGGX(N, H, roughness);
        // 几何函数
        float G   = GeometrySmith(N, V, L, roughness);
        // 菲涅尔方程
        vec3 F  = fresnelSchlick(clamp(dot(H, V), 0.0, 1.0), F0);

        float NdotL = max(dot(N, L), 0.0);  

        vec3 numerator = NDF * G * F;
        float denominator = 4.0 * max(dot(N, V), 0.0) * NdotL; 
        vec3 specular     = numerator / max(denominator,0.001); // 镜面反射项

        vec3 kS = F;
        vec3 kD = (vec3(1.0) - kS);
        kD *= (1.0 - metallic);

        Lo += (kD * albedo / PI+ specular) * radiance * NdotL;
    }
    vec3 ambient = vec3(0.03) * albedo * ao;
    vec3 color   = ambient + Lo;

    // 色调映射
    color = color / (color + vec3(1.0));
    color.x = pow(color.x, 1.0/2.2); 
    color.y = pow(color.y, 1.0/2.2); 
    color.z = pow(color.z, 1.0/2.2); 
    outColor = vec4(color,1);
}