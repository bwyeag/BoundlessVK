#version 460
#pragma shader_stage(vertex)

layout(binding = 0, set=0) uniform trianglePosition {
    mat4 model[3];
    mat4 cameraMat;
};

layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main() {
    gl_Position = cameraMat*model[gl_InstanceIndex]*vec4(inPosition, 0, 1);
    outColor = inColor;
}