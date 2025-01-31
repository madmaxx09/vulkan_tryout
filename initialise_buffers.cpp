#include "initialise_buffers.hpp"

namespace wind
{
	void initialise_buffer(t_buffer &buffer, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryFlags, EngineDevice &device, VkDeviceSize bufferSize)
	{
		device.createBuffer(
			bufferSize,
			usageFlags,
			memoryFlags,
			buffer.buffer,
			buffer.memory
		);
	}

	void destroy_buffer(t_buffer &buffer, EngineDevice &device)
	{
		vkDestroyBuffer(device.device(), buffer.buffer, nullptr);
		vkFreeMemory(device.device(), buffer.memory, nullptr);
	}
}
