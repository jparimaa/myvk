#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform TransformationMatrices
{
    mat4 world;
    mat4 view;
    mat4 proj;
}
matrices;

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec3 outUv;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    gl_Position = (matrices.proj * matrices.view * matrices.world * vec4(inPos, 1.0)).xyww;
    outUv = inPos;
}
