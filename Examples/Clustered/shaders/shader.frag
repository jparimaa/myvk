#version 450
#extension GL_ARB_separate_shader_objects : enable

const float c_gridDepthValues[5] = {1.0, 5.0, 20.0, 50.0, 100.0};
const int c_workgroupSizeX = 32;
const int c_workgroupSizeY = 30;
// For demo purposes a fixed resolution helps a bit
const int pixelsPerXTile = 1600 / c_workgroupSizeX; // 1600 / 32 = 50
const int pixelsPerYTile = 1200 / c_workgroupSizeY; // 1200 / 30 = 40

layout(binding = 1) uniform sampler2D albedo;

struct Light
{
	vec4 position;
	vec4 color;
};

layout(std430, binding = 2) readonly buffer lightBuffer 
{
   Light lights[];
};

layout(std430, binding = 3) readonly buffer tileBuffer 
{
   uint lightIndex[];
};

layout(std430, binding = 4) readonly buffer numLightsPerTileBuffer 
{
	uint numLights[];
};

layout(std140, binding = 5) readonly uniform sceneInfo
{
	float ncp;
	float fcp;
	uint lightCount;
	uint maxLightsPerTile;
} scene;

layout(location = 0) in vec2 inUv;
layout(location = 1) in float depth;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inPosWorld;

layout(location = 0) out vec4 outColor;

void main()
{
   vec3 albedo = texture(albedo, inUv).xyz;
   int x = int(gl_FragCoord.x / pixelsPerXTile);
   int y = int(gl_FragCoord.y / pixelsPerYTile);
   
   int z = 0;
   while (depth > c_gridDepthValues[z+1])
   {
      ++z;
   }

   int numLightsIndex = (z * c_workgroupSizeX * c_workgroupSizeY) + (y * c_workgroupSizeX) + x;
   uint lightCount = numLights[numLightsIndex];
   uint offset = (z * c_workgroupSizeX * c_workgroupSizeY * scene.maxLightsPerTile) 
	+ (y * c_workgroupSizeX * scene.maxLightsPerTile) 
	+ (x * scene.maxLightsPerTile);

   vec3 diff = vec3(0.0);
   for (uint i = 0; i < lightCount; ++i)
   {
      uint lightTileIndex = offset + i;
      uint index = lightIndex[lightTileIndex];
      Light light = lights[index];

      vec3 L = light.position.xyz - inPosWorld;
      float dist = length(L);
      L = normalize(L);
      float attenuation = max(light.position.w - dist, 0.0);
      vec3 N = normalize(inNormal);
      float NdotL = max(0.0, dot(N, L));
      diff += albedo * NdotL * attenuation * light.color.rgb * light.color.a;
   }
   outColor.rgb = diff;
}
