#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;

// These constant are updated from the C++-side with VkSpecializationInfo
layout (constant_id = 0) const float cGrayscale = 0.0;
layout (constant_id = 1) const float cColorMultiplier = 9.0;

void main()
{
    outColor = texture(albedo, inUv) * cColorMultiplier;
    if (cGrayscale > 0.0)
    {
        float gray = dot(outColor.rgb, vec3(0.299, 0.587, 0.114));
        outColor = vec4(vec3(gray), 1.0);
    }
}
