#version 450

layout(set = 0, binding = 0) uniform sampler2D preLightShaft;

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outColor;

void main()
{
    outColor = texture(preLightShaft, inUv);
}
