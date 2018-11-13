#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;

layout (location = 0) in vec4 outViewPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

void main()
{
    outPosition = vec4(outViewPos.xyz, 1.0);
	outPosition.z = -outPosition.z;

	vec3 N = normalize(inNormal);
	outNormal = vec4(N, 1.0);

	outAlbedo = texture(albedo, inUv);
}
