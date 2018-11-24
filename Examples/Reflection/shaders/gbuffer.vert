#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform TransformationMatrices
{
    mat4 world;
    mat4 view;
    mat4 proj;
}
matrices;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;

layout (location = 0) out vec4 outViewPos;
layout (location = 1) out vec2 outUv;
layout (location = 2) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outViewPos = matrices.view * matrices.world * vec4(inPosition, 1.0);
    outUv = inUv;
    outNormal = mat3(matrices.view) * mat3(matrices.world) * normalize(inNormal);
    gl_Position = matrices.proj * outViewPos;
}
