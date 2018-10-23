#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform ColorData
{
    vec4 color;
}
colorData;

layout(push_constant) uniform PushConsts {
	vec4 color;
} pushConsts;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = pushConsts.color;
}
