#version 450

layout(binding = 0) uniform samplerCube environment;

layout(location = 0) in vec3 inPos;

layout(location = 0) out vec4 outColor;

layout(push_constant) uniform PushConsts
{
    layout(offset = 64) float deltaPhi;
    layout(offset = 68) float deltaTheta;
}
consts;

const float PI = 3.1415926536;
const float TWO_PI = 6.283185307;
const float HALF_PI = 1.570796327;

void main()
{
    vec3 N = normalize(inPos);
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(up, N));
    up = cross(N, right);

    vec3 color = vec3(0.0);
    uint sampleCount = 0u;
    for (float phi = 0.0; phi < TWO_PI; phi += consts.deltaPhi) {
        for (float theta = 0.0; theta < HALF_PI; theta += consts.deltaTheta) {
            vec3 tempVec = cos(phi) * right + sin(phi) * up;
            vec3 sampleVector = cos(theta) * N + sin(theta) * tempVec;
            color += texture(environment, sampleVector).rgb * cos(theta) * sin(theta);
            sampleCount++;
        }
    }
    outColor = vec4(PI * color / float(sampleCount), 1.0);
}
