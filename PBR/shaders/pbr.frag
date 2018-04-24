#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform sampler2D diffuse;
layout(set = 0, binding = 2) uniform sampler2D emissive;
layout(set = 0, binding = 3) uniform sampler2D normals;
layout(set = 0, binding = 4) uniform sampler2D lightmap;
layout(set = 0, binding = 5) uniform samplerCube irradiance;
layout(set = 0, binding = 6) uniform samplerCube prefilter;
layout(set = 0, binding = 7) uniform sampler2D brdfLut;

layout(location = 0) in vec3 modelPosition;
layout(location = 1) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(diffuse, texCoord);
    outColor += texture(emissive, texCoord);
    outColor *= texture(lightmap, texCoord);
    outColor += texture(prefilter, modelPosition) * 0.2;
    outColor += texture(brdfLut, texCoord);
}
