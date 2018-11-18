#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform sampler2D albedo;

layout (location = 0) in vec4 inViewPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;

void main()
{
    outPosition = vec4(inViewPos.xyz, 1.0);
	outPosition.z = -outPosition.z;

	vec3 N = normalize(inNormal);
	outNormal = vec4(N, 1.0);

	outAlbedo = texture(albedo, inUv);
	//outAlbedo.a = reflectivity;
}
