#version 450

layout (location = 0) in vec3 fragColor;

layout (location = 0) out vec4 outColor; //layout nous dis ou cette variable va etre output out vec4 défini son type et outColor est le nom de ce "type" de variable


void main()
{
	outColor = vec4(fragColor, 1.0);
}