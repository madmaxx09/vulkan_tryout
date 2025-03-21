#version 450

const vec2 OFFSETS[6] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, -1.0),
	vec2(1.0, -1.0),
	vec2(-1.0, 1.0),
	vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;

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
	vec4 position;
	vec4 color;
	float radius;
} push;

const float LIGHT_RADIUS = 0.1;

void main()
{
	fragOffset = OFFSETS[gl_VertexIndex];
	
	vec3 cameraUpWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
	vec3 cameraRightWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

	vec3 positionWorld = push.position.xyz 
		+ push.radius * fragOffset.x * cameraRightWorld
		+ push.radius * fragOffset.y * cameraUpWorld;


	gl_Position = ubo.projection * ubo.view * vec4(positionWorld, 1.0);

	// vec4 lightInCameraSpace = ubo.view * vec4(ubo.lightPosition, 1.0);
	// vec4 positionInCameraSpace = lightInCameraSpace + LIGHT_RADIUS * vec4(fragOffset, 0.0, 0.0);

	// gl_Position = ubo.projection * positionInCameraSpace;
}