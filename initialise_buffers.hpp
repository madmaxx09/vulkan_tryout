#pragma once

#include "engine.hpp"
#include <cstring>


namespace wind
{
	typedef struct s_buffer
	{
		VkBuffer buffer = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		void* data = nullptr;
	} t_buffer;

	void initialise_buffer(t_buffer &buffer, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, EngineDevice &device, VkDeviceSize bufferSize);
	void destroy_buffer(t_buffer &buffer, EngineDevice &device);
}