#version 450

layout(set = 0, binding = 0) uniform sampler2D tex1;
layout(set = 0, binding = 1) uniform sampler2D tex2;
layout(set = 0, binding = 2) uniform sampler2D tex3;

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outColor;

void main()
{    
    outColor = (texture(tex1, inUv) * 0.3) + (texture(tex2, inUv) * 0.3) + (texture(tex3, inUv) * 0.3);
}
