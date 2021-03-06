#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform TransformationMatrices
{
    mat4 world;
    mat4 view;
    mat4 proj;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inUv;

layout (location = 0) out vec4 outWorldPos;
layout (location = 1) out vec2 outUv;
layout (location = 2) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    outWorldPos = ubo.world * vec4(inPosition, 1.0);
    outUv = inUv;
	outNormal = transpose(inverse(mat3(ubo.world))) * normalize(inNormal);
    gl_Position = ubo.proj * ubo.view * outWorldPos;
}
