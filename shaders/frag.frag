#version 450

layout (location = 0) out vec4 outColor; //layout nous dis ou cette variable va etre output out vec4 défini son type et outColor est le nom de ce "type" de variable

layout(push_constant) uniform Push {
	mat2 transform;
	vec2 offset;
	vec3 color;
} push;

void main()
{
	outColor = vec4(push.color, 1.0);
}