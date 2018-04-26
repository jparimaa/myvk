#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform sampler2D albedo;
layout(set = 0, binding = 2) uniform sampler2D metallic;
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

void main()
{
    outColor = texture(albedo, texCoord) * 0.01;
    outColor += texture(metallic, texCoord);
    outColor += texture(emissive, texCoord);
    outColor *= texture(lightmap, texCoord);
    outColor += texture(prefilter, worldPosition) * 0.01;
    outColor += texture(brdfLut, texCoord) * 0.01;
    outColor.rgb += normal;
}
