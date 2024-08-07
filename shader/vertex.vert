#version 460
#pragma shader_stage(vertex)

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

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 0) out vec3 outColor;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec4 outFragPos;

void main() {
    outFragPos = model*vec4(inPosition,1);
    outColor = ambientStrength*lightColor.xyz*objectColor.xyz;
    outNormal = transNormal * inNormal;
    gl_Position = cameraMat * outFragPos;
}