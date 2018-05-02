#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform TransformationMatrices {
    mat4 world;
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
}
ubo;
layout(set = 0, binding = 1) uniform sampler2D albedo;
layout(set = 0, binding = 2) uniform sampler2D metallicRoughness;
layout(set = 0, binding = 3) uniform sampler2D emissive;
layout(set = 0, binding = 4) uniform sampler2D normals;
layout(set = 0, binding = 5) uniform sampler2D lightmap;
layout(set = 0, binding = 6) uniform samplerCube irradiance;
layout(set = 0, binding = 7) uniform samplerCube prefilter;
layout(set = 0, binding = 8) uniform sampler2D brdfLut;

layout(location = 0) in vec3 worldPosition;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

#define PI 3.1415926535897932384626433832795
#define ALBEDO pow(texture(albedo, texCoord).rgb, vec3(2.2))
#define LIGHT_DIR vec3(0.0, -0.5, -0.5)
#define EXPOSURE 4.5
#define GAMMA 2.2

// From http://filmicworlds.com/blog/filmic-tonemapping-operators/
vec3 Uncharted2Tonemap(vec3 x)
{
	float A = 0.15;
	float B = 0.50;
	float C = 0.10;
	float D = 0.20;
	float E = 0.02;
	float F = 0.30;
	return ((x*(A*x+C*B)+D*E)/(x*(A*x+B)+D*F))-E/F;
}

// Normal Distribution function
float D_GGX(float dotNH, float roughness)
{
	float alpha = roughness * roughness;
	float alpha2 = alpha * alpha;
	float denom = dotNH * dotNH * (alpha2 - 1.0) + 1.0;
	return (alpha2)/(PI * denom*denom);
}

// Geometric Shadowing function
float G_SchlicksmithGGX(float dotNL, float dotNV, float roughness)
{
	float r = (roughness + 1.0);
	float k = (r*r) / 8.0;
	float GL = dotNL / (dotNL * (1.0 - k) + k);
	float GV = dotNV / (dotNV * (1.0 - k) + k);
	return GL * GV;
}

// Fresnel function
vec3 F_Schlick(float cosTheta, vec3 F0)
{
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 F_SchlickR(float cosTheta, vec3 F0, float roughness)
{
	return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 prefilteredReflection(vec3 R, float roughness)
{
	const float MAX_REFLECTION_LOD = 9.0;
	float lod = roughness * MAX_REFLECTION_LOD;
	float lodf = floor(lod);
	float lodc = ceil(lod);
	vec3 a = textureLod(prefilter, R, lodf).rgb;
	vec3 b = textureLod(prefilter, R, lodc).rgb;
	return mix(a, b, lod - lodf);
}

vec3 specularContribution(vec3 L, vec3 V, vec3 N, vec3 F0, float metallic, float roughness)
{
	// Precalculate vectors and dot products
	vec3 H = normalize (V + L);
	float dotNH = clamp(dot(N, H), 0.0, 1.0);
	float dotNV = clamp(dot(N, V), 0.0, 1.0);
	float dotNL = clamp(dot(N, L), 0.0, 1.0);

	// Light color fixed
	vec3 lightColor = vec3(1.0);

	vec3 color = vec3(0.0);

	if (dotNL > 0.0) {
		// D = Normal distribution (Distribution of the microfacets)
		float D = D_GGX(dotNH, roughness);
		// G = Geometric shadowing term (Microfacets shadowing)
		float G = G_SchlicksmithGGX(dotNL, dotNV, roughness);
		// F = Fresnel factor (Reflectance depending on angle of incidence)
		vec3 F = F_Schlick(dotNV, F0);
		vec3 spec = D * F * G / (4.0 * dotNL * dotNV + 0.001);
		vec3 kD = (vec3(1.0) - F) * (1.0 - metallic);
		color += (kD * ALBEDO / PI + spec) * dotNL;
	}

	return color;
}

void main()
{
	vec3 N = normalize(normal);
	vec3 V = normalize(ubo.cameraPosition - worldPosition);
	vec3 R = reflect(-V, N);

	float metallic = texture(metallicRoughness, texCoord).b;
	float roughness = texture(metallicRoughness, texCoord).g;

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, ALBEDO, metallic);

	vec3 L = normalize(LIGHT_DIR); // Light dir
	vec3 Lo = specularContribution(L, V, N, F0, metallic, roughness);

	vec2 brdf = texture(brdfLut, vec2(max(dot(N, V), 0.0), roughness)).rg;
	vec3 reflection = prefilteredReflection(R, roughness).rgb;
	vec3 irradiance = texture(irradiance, N).rgb;

	// Diffuse based on irradiance
	vec3 diffuse = irradiance * ALBEDO;

	vec3 F = F_SchlickR(max(dot(N, V), 0.0), F0, roughness);

	// Specular reflectance
	vec3 specular = reflection * (F * brdf.x + brdf.y);

	// Ambient part
	vec3 kD = 1.0 - F;
	kD *= 1.0 - metallic;
	float ao = texture(lightmap, texCoord).r;
	vec3 ambient = (kD * diffuse + specular) * ao;
	vec3 color = ambient + Lo;

	// Tone mapping
	color = Uncharted2Tonemap(color * EXPOSURE);
	color = color * (1.0f / Uncharted2Tonemap(vec3(11.2f)));
	// Gamma correction
	color = pow(color, vec3(1.0f / GAMMA));

    vec3 emissive = texture(emissive, texCoord).rgb;
    color += emissive;

	outColor = vec4(color, 1.0);
}
