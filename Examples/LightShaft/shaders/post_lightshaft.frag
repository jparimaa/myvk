#version 450

layout(push_constant) uniform PushConsts {
	vec2 lightPosScreen;
} pushConsts;

layout(set = 0, binding = 0) uniform sampler2D preLightShaft;

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outColor;

int c_numSamples = 100;
float c_density = 0.2;
float c_weight = 0.8;
float c_decay = 0.98;
float c_exposure = 0.15;

void main()
{
    vec2 deltaTexCoord = inUv - pushConsts.lightPosScreen;  
    deltaTexCoord *= 1.0f / c_numSamples * c_density;

    vec2 texCoord = inUv;
    vec3 color = texture(preLightShaft, texCoord).rgb;

    float illuminationDecay = 1.0f;

    for (int i = 0; i < c_numSamples; ++i)
    {
        texCoord -= deltaTexCoord;

        vec3 colorSample = texture(preLightShaft, texCoord).rgb;
        colorSample *= illuminationDecay * c_weight;

        color += colorSample;
        illuminationDecay *= c_decay;
    }
    outColor = vec4(color * c_exposure, 1.0);
}
