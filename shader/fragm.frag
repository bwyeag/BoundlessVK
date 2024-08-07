#version 460
#pragma shader_stage(fragment)

layout(binding = 0, set=0) uniform uniformData {
    vec4 lightColor;
    vec4 lightPosition;
    float ambientStrength;
    float specularStrength;

    vec4 objectColor;
    vec4 cameraPos;
    mat4 cameraMat;
    mat4 model;
    mat3 transNormal;
};

layout(location = 0) in vec3 inColor; // 已经完成环境光计算
layout(location = 1) smooth in vec3 inNormal;
layout(location = 2) in vec4 inFragPos;
layout(location = 0) out vec4 outColor;

void main() {
    // 计算漫反射光照
    vec3 norm = normalize(inNormal);
    vec3 lightVec = lightPosition.xyz - inFragPos.xyz;
    float lightDistance =  lightVec.length();
    vec3 lightDir = lightVec / lightDistance;
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor.xyz;
    // 计算镜面反射光照
    vec3 viewDir = (normalize(cameraPos - inFragPos)).xyz;
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 16);
    vec3 specular = specularStrength * spec * lightColor.xyz;

    outColor = vec4(specular+diffuse+inColor, 1.0);
}