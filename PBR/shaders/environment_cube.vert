#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform PushConsts {
    layout(offset = 0) mat4 mvp;
}
pushConsts;

layout(location = 0) out vec3 pos;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    pos = inPosition;
    gl_Position = pushConsts.mvp * vec4(inPosition, 1.0);
}
