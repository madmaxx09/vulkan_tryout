#pragma once

#include "camera.hpp"
#include "engine.hpp"

namespace wind
{
	typedef struct s_frame_info
	{
		int				frameIndex;
		float			frameTime;
		VkCommandBuffer	commandBuffer;
		LveCamera		&camera;
	} t_frame_info;
	
}