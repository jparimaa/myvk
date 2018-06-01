#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (input_attachment_index = 0, binding = 0) uniform subpassInput position;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput normal;
layout (input_attachment_index = 2, binding = 2) uniform subpassInput albedo;

layout (location = 0) in vec2 inUv;

layout (location = 0) out vec4 outColor;

const vec3 LIGHT_POS = vec3(2.0, 2.0, 2.0);
const float LIGHT_RADIUS = 30.0;

void main()
{
	// Read G-Buffer values from previous sub pass
	vec3 fragPos = subpassLoad(position).rgb;
	vec3 normal = subpassLoad(normal).rgb;
	vec4 albedo = subpassLoad(albedo);

	vec3 fragColor = albedo.rgb * 0.1;

    vec3 L = LIGHT_POS - fragPos;
    float dist = length(L);

    L = normalize(L);

    float attenuation = LIGHT_RADIUS / (pow(dist, 2.0) + 1.0);

    vec3 N = normalize(normal);
    float NdotL = max(0.0, dot(N, L));
    vec3 diff = albedo.rgb * NdotL * attenuation;

    fragColor += diff;

	outColor = vec4(fragColor, 1.0);
}
