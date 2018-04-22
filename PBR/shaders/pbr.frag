#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 1) uniform sampler2D diffuse;
layout(set = 0, binding = 2) uniform sampler2D emissive;
layout(set = 0, binding = 3) uniform sampler2D normals;
layout(set = 0, binding = 4) uniform sampler2D lightmap;

layout(location = 0) in vec2 texCoord;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(diffuse, texCoord);
    outColor += texture(emissive, texCoord);
    outColor *= texture(lightmap, texCoord);
}
