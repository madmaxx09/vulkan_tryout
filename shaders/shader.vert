#version 450 

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragWorldPos;
layout(location = 2) out vec3 fragWorldNormal;

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
	vec4 vertexWorldSpace = push.modelMatrix * vec4(position, 1.0);
	gl_Position = ubo.projection * ubo.view * vertexWorldSpace;

	fragWorldNormal = normalize(mat3(push.normalMatrix) * normal);
	fragWorldPos = vertexWorldSpace.xyz;
	fragColor = color;
}

