#pragma once

#include "camera.hpp"
#include "engine.hpp"
#include "game_object.hpp"
#include <vector>

#define MAX_LIGHTS 10

namespace wind
{
	struct PointLight
	{
		glm::vec4 position{};
		glm::vec4 color{}; //fourth param for intensity
	};

	//meh using vec4 for colors instead of vec3 where color scaling is on xyz might be heavier for no reason idk
	struct GlobalUBO //if something does not work always check alignement of ubo into shader (std140) 
	{
		glm::mat4	projection{1.f};
		glm::mat4	view{1.f};
		glm::vec4	ambientLight{1.f, 1.f, 1.f, 0.02f}; //w is light intensity
		
		PointLight pointLights[MAX_LIGHTS];
		int lightCount;
	};

	typedef struct s_frame_info
	{
		int				frameIndex;
		float			frameTime;
		VkCommandBuffer	commandBuffer;
		LveCamera		&camera;
		VkDescriptorSet	globalDescriptorSet;
		LveGameObject::Map &gameObjects;
	} t_frame_info;
	
}