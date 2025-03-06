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
	mat4 inverseView;
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
	vec3 specularLight = vec3(0.0);
	vec3 surfaceNormal = normalize(fragWorldNormal);

	vec3 cameraPosWorld = ubo.inverseView[3].xyz;
	vec3 viewDirection = normalize(cameraPosWorld - fragWorldPos);

	for (int i = 0; i < ubo.lightCount; i++)
	{
		PointLight light = ubo.pointLights[i];
		vec3 directionToLight = light.position.xyz - fragWorldPos;
		float attenuation = 1.0 / dot(directionToLight, directionToLight); //this is equivalent to real world physics formula for light attenuation which is (1 / distance²)
		directionToLight = normalize(directionToLight);

		float cosAngInc = max(dot(normalize(fragWorldNormal), directionToLight), 0); //value can be neg if surface is opposite to light dir
		vec3 intensity = light.color.xyz * light.color.w * attenuation;

		diffuseLight += intensity * cosAngInc;

		//specular calculation
		vec3 halfAngle = normalize(directionToLight + viewDirection);
		float blinnTerm = dot(surfaceNormal, halfAngle);
		blinnTerm = clamp(blinnTerm, 0, 1);
		blinnTerm = pow(blinnTerm, 32.0); //higher val == sharper highlight, this value should be passed to the shaders per object
		specularLight += intensity * blinnTerm;
	}
 

	outColor = vec4(diffuseLight * fragColor + specularLight * fragColor, 1.0);
}