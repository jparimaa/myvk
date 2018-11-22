#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform sampler2D albedo;

layout(push_constant) uniform PushConsts {
	float reflectivity;
};

layout (location = 0) in vec4 inViewPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outAlbedo;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;

void main()
{
    outPosition = inViewPos;

	vec3 N = normalize(inNormal);
	outNormal = vec4(N, 1.0);

	outAlbedo = texture(albedo, inUv);
	outAlbedo.a = reflectivity;
}
