#version 460
#pragma shader_stage(fragment)

layout(location = 0) in vec4 inColor;
layout(location = 0) out vec4 outColor;

void main() {
    float p = pow(gl_FragCoord.z,10);
    outColor = vec4(p,p,p,1.0f);
}