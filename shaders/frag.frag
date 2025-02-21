#version 450

layout (location = 0) in vec3 fragColor;
layout (location = 1) in vec3 fragWorldPos;
layout (location = 2) in vec3 fragWorldNormal;

layout (location = 0) out vec4 outColor; //layout nous dis ou cette variable va etre output out vec4 défini son type et outColor est le nom de ce "type" de variable

struct PointLight {
	vec4 position;
	vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projection;
	mat4 view;
	vec4 ambientLight;
	PointLight pointLights[10]; //look into speciliazition constants
	int lightCount;
} ubo;

layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;

void main()
{
	vec3 diffuseLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
	vec3 surfaceNormal = normalize(fragWorldNormal);

	for (int i = 0; i < ubo.lightCount; i++)
	{
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragWorldPos;
		float attenuation = 1.0 / dot(directionToLight, directionToLight); //this is equivalent to real world physics formula for light attenuation which is (1 / distance²)
		float cosAngInc = max(dot(normalize(fragWorldNormal), normalize(directionToLight)), 0); //value can be neg if surface is opposite to light dir
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngInc;
	}
 

	outColor = vec4(diffuseLight * fragColor, 1.0);
}