#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;

layout(push_constant) uniform PushConsts {
	vec4 color;
} pushConsts;

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(albedo, inUv) * pushConsts.color;
}
