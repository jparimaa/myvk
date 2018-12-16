#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform TransformationMatrices
{
    mat4 world;
    mat4 view;
    mat4 proj;
}
ubo;

layout(location = 0) in vec4 inPosition;
layout(location = 1) in vec4 inDirection;

layout(location = 0) out float outSpeed;

layout (constant_id = 0) const float c_initialSpeed = 100.0;

out gl_PerVertex
{
    vec4 gl_Position;
   	float gl_PointSize;
};

void main()
{
    gl_PointSize = 2.0;
    gl_Position = ubo.proj * ubo.view * vec4(inPosition.xyz, 1.0);
    outSpeed = length(inDirection.xyz) * (1.0 / c_initialSpeed);
}
