#pragma once

#include "camera.hpp"
#include "engine.hpp"
#include "game_object.hpp"

namespace wind
{
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