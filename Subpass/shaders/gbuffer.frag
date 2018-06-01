#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;

layout (location = 0) in vec4 inWorldPos;
layout (location = 1) in vec2 inUv;
layout (location = 2) in vec3 inNormal;

layout (location = 0) out vec4 outColor;
layout (location = 1) out vec4 outPosition;
layout (location = 2) out vec4 outNormal;
layout (location = 3) out vec4 outAlbedo;

void main()
{
    // Write color attachments to avoid undefined behaviour (validation error)
	outColor = vec4(0.0);

    outPosition = vec4(inWorldPos.xyz, 1.0);

	vec3 N = normalize(inNormal);
	outNormal = vec4(N, 1.0);

	outAlbedo = texture(albedo, inUv);;
}
