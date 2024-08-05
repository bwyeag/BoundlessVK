#version 460
#pragma shader_stage(vertex)

layout(binding = 0, set=0) uniform trianglePosition {
    mat4 model[3];
    mat4 cameraMat;
};

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 0) out vec4 outColor;

void main() {
    gl_Position = cameraMat*model[gl_InstanceIndex]*vec4(inPosition,1);
    outColor = vec4(inNormal.xxx,1.0f);
}