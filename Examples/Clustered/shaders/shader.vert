#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform TransformationMatrices
{
    mat4 world;
    mat4 view;
    mat4 proj;
    mat4 inverseProj;
}
matrices;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;

layout(location = 0) out vec2 outUv;
layout(location = 1) out float depth;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outPosWorld;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    vec4 posWorld = matrices.world * vec4(inPosition, 1.0);
    outPosWorld.xyz = posWorld.xyz;
    vec4 view = matrices.view * posWorld;
    gl_Position = matrices.proj * view;
   	outNormal = transpose(inverse(mat3(matrices.world))) * normalize(inNormal);
    outUv = inUv;
    depth = -view.z;
}
