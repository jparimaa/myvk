#version 450

layout(push_constant) uniform PushConsts {
	vec2 lightPosScreen;
    int numSamples;
    float density;
    float weight;
    float decay;
    float exposure;
} pushConsts;

layout(set = 0, binding = 0) uniform sampler2D preLightShaft;
layout(set = 0, binding = 1) uniform sampler2D objectTexture;

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outColor;

void main()
{
    vec2 deltaTexCoord = inUv - pushConsts.lightPosScreen;  
    deltaTexCoord *= 1.0f / pushConsts.numSamples * pushConsts.density;
    
    vec2 texCoord = inUv;
    vec3 color = texture(preLightShaft, texCoord).rgb;

    float illuminationDecay = 1.0f;

    for (int i = 0; i < pushConsts.numSamples; ++i)
    {
        texCoord -= deltaTexCoord;

        vec3 colorSample = texture(preLightShaft, texCoord).rgb;
        colorSample *= illuminationDecay * pushConsts.weight;

        color += colorSample;
        illuminationDecay *= pushConsts.decay;
    }
    outColor = vec4(color * pushConsts.exposure, 1.0);
    outColor += texture(objectTexture, inUv);
}
