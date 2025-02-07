#version 450 

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUBO {
	mat4 projectionView;
	vec4 ambientLight;
	vec3 lightPosition;
	vec4 lightColor;
} ubo;


layout(push_constant) uniform Push {
	mat4 modelMatrix;
	mat4 normalMatrix;
} push;


void main()
{
	vec4 vertexWorldSpace = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projectionView * vertexWorldSpace;

	vec3 normalWorldSpace = normalize(mat3(push.normalMatrix) * normal);

	vec3 directionToLight = ubo.lightPosition - vertexWorldSpace.xyz;
	float attenuation = 1.0 / dot(directionToLight, directionToLight); //this is equivalent to real world physics formula for light attenuation which is (1 / distance²)

	vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w * attenuation;
	vec3 ambientLight = ubo.ambientLight.xyz * ubo.ambientLight.w;
	vec3 diffuseLight = lightColor * max(dot(normalWorldSpace, normalize(directionToLight)), 0); //value can be neg if surface is opposite to light dir
 

	fragColor = (diffuseLight + ambientLight) * color;
}

