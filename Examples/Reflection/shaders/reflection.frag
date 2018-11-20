#version 450

layout(set = 0, binding = 0) uniform sampler2D albedo;
layout(set = 0, binding = 1) uniform sampler2D position;
layout(set = 0, binding = 2) uniform sampler2D normal;

layout(push_constant) uniform PushConsts {
	mat4 projection;
};

layout (location = 0) in vec2 inUv;
layout (location = 0) out vec4 outColor;

const float rayStep = 0.01;
const float maxSteps = 1000;

vec3 raycast(vec3 dir, vec3 hitCoord)
{
    dir *= rayStep;
 
    for (int i = 0; i < maxSteps; ++i)
    {
        hitCoord += dir;
 
        vec4 projectedCoord = projection * vec4(hitCoord, 1.0);
        projectedCoord.xy /= projectedCoord.w;
        projectedCoord.xy = projectedCoord.xy * 0.5 + 0.5;
 
        float depth = texture(position, projectedCoord.xy).z;
  
        if ((depth - hitCoord.z) < 0.0)
        {
            return vec3(projectedCoord.xy, 1.0);
        }
    }
 
    return vec3(0.0);
}

void main()
{    
    vec3 viewNormal = texture(normal, inUv).xyz;
    vec4 viewPos = texture(position, inUv);
    vec4 color = texture(albedo, inUv);

    vec3 reflected = normalize(reflect(normalize(viewPos.xyz), normalize(viewNormal)));
    vec3 hitCoordinates = raycast(reflected, viewPos.xyz);

    float diffuseAmount = 1.0 - color.a;
    vec4 reflectionColor = vec4(0.0);
    if (hitCoordinates.z > 0.5)
    {
        reflectionColor = texture(albedo, hitCoordinates.xy);
        reflectionColor.rgb *= color.a;        
    }

    outColor.rgb = color.rgb * diffuseAmount + reflectionColor.rgb;    
    outColor.a = 1.0;
}
