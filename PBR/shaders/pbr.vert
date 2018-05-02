#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform TransformationMatrices {
    mat4 world;
    mat4 view;
    mat4 proj;
    vec3 cameraPosition;
}
ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec2 outTexCoord;

out gl_PerVertex {
    vec4 gl_Position;
};

void main()
{
    outWorldPosition = vec3(ubo.world * vec4(inPosition, 1.0));
    outNormal = mat3(ubo.world) * inNormal;
    outTexCoord = inTexCoord;
    gl_Position = ubo.proj * ubo.view * ubo.world * vec4(inPosition, 1.0);
}
