#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform GlobalMatrices
{
    mat4 view;
    mat4 proj;
}
global;

layout(binding = 2) uniform LocalMatrices
{
    mat4 world;
}
local;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec2 outUv;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = global.proj * global.view * local.world * vec4(inPosition, 1.0);
    outUv = inUv;
}
