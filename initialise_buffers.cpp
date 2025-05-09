#include "initialise_buffers.hpp"
#include <iostream>

namespace wind
{
	//last argument is optionnal it allows double or more buffering with the same buffer object
	void initialise_buffer(t_buffer &buffer, VkBufferUsageFlags usageFlags,
		VkMemoryPropertyFlags memoryFlags, EngineDevice &device,
		VkDeviceSize bufferSize)
	{
		device.createBuffer(
			bufferSize,
			usageFlags,
			memoryFlags,
			buffer.buffer,
			buffer.memory
		);
		//std::cout << "does buffer == buffer : " << buffer.buffer << std::endl;
 	}

	void destroy_buffer(t_buffer &buffer, EngineDevice &device)
	{
		vkDestroyBuffer(device.device(), buffer.buffer, nullptr);
		vkFreeMemory(device.device(), buffer.memory, nullptr);
	}
}
