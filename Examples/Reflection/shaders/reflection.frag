#version 450

layout(set = 0, binding = 0) uniform sampler2D albedo;
layout(set = 0, binding = 1) uniform sampler2D position;
layout(set = 0, binding = 2) uniform sampler2D normal;
layout(set = 0, binding = 3) uniform ProjectionMatrix
{
    mat4 projectionMatrix;
};

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outColor;

const float rayStep = 0.007;
const float maxSteps = 1000;

vec3 raycast(vec3 dir, vec3 hitCoord)
{
    dir *= rayStep;
 
    for (int i = 0; i < maxSteps; ++i)
    {
        hitCoord += dir;
 
        vec4 projectedCoord = projectionMatrix * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;

        float depth = texture(position, projectedCoord.xy).z;

        if (depth > (hitCoord.z + 0.01))
        {
            return vec3(projectedCoord.xy, (maxSteps - i) / maxSteps);
        }
    }
 
    return vec3(0.0);
}

void main()
{    
    vec3 viewNormal = texture(normal, inUv).xyz;
    vec4 viewPos = texture(position, inUv);
    vec4 color = texture(albedo, inUv);
    float reflectivity = color.a;
    if (reflectivity == 0.0)
    {
        outColor.rgb = color.rgb;
        outColor.a = 1.0;
    }
    else
    {
        vec3 reflected = normalize(reflect(viewPos.xyz, normalize(viewNormal)));
        vec3 hitCoordinates = raycast(reflected, viewPos.xyz);                
        vec4 reflectionColor = texture(albedo, hitCoordinates.xy) * hitCoordinates.z;

        float diffuseAmount = 1.0 - reflectivity;
        outColor.rgb = color.rgb * diffuseAmount + reflectionColor.rgb * reflectivity;
        outColor.a = 1.0;
    }
    
}
