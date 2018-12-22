#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1) uniform sampler2D albedo;

struct Light
{
	vec4 position;
	vec4 color;
};

layout(std140, binding = 2) buffer lightBuffer 
{
   Light lights[];
};

layout(std140, binding = 3) buffer lightIndexBuffer 
{
   uint lightIndex[];
};

struct Tile
{
	uint offset;
	uint count;
};

layout(std140, binding = 4) buffer tileBuffer 
{
   Tile tiles[];
};

layout(location = 0) in vec2 inUv;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(albedo, inUv);
}
