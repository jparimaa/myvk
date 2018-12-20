#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;

layout(location = 0) in float inSpeed;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(0.0, 1.0 - inSpeed, 1.0, 1.0);
}
