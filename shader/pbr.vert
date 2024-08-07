#version 460
#pragma shader_stage(vertex)

layout(constant_id = 1) const uint MAX_INSTANCE_COUNT = 8;

struct InstanceData {
    mat4 matrixModel;
    mat3 matrixNormal;
    vec3  albedo;
    float metallic;
    float roughness;
    float ao;
};
layout(binding = 1, set = 0) uniform SenceUniform {
    vec4 cameraPosition;
    mat4 matrixCamera;
    InstanceData instance[MAX_INSTANCE_COUNT];
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;

layout(location = 0) smooth out vec3 outNormal;
layout(location = 1) out vec3 outWorldPos;
layout(location = 2) flat out uint passIndex;

void main() {
    vec4 position = instance[gl_InstanceIndex].matrixModel * vec4(inPosition,1);
    outWorldPos = position.xyz;
    outNormal = instance[gl_InstanceIndex].matrixNormal * inNormal;
    passIndex = gl_InstanceIndex;
    gl_Position = matrixCamera * position;
}