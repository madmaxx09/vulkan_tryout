#include "model.hpp"

#include <cstring>
#include <cassert>

namespace wind
{
	LveModel::LveModel(EngineDevice &device, const std::vector<Vertex> &vertices) : device{device}
	{
		createVertexBuffers(vertices);
	}

	LveModel::~LveModel()
	{
		vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
		vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
	}

	void LveModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer	buffers[] = {vertexBuffer};
		VkDeviceSize	offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void LveModel::draw(VkCommandBuffer commandBuffer)
	{
		vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count should be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;
		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, //precise que notre buffer contiendra des infos de vertex
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, //fait en sorte que 
			vertexBuffer,
			vertexBufferMemory);
		
		void *data;
		vkMapMemory(device.device(), vertexBufferMemory, 0, bufferSize, 0, &data); //map une partie de la mémoire du Cpu pour matcher la mémoire du gpu dans enginedevice
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.device(), vertexBufferMemory);
	}

	std::vector<VkVertexInputBindingDescription> LveModel::Vertex::getBindingDescriptions()
	{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions; 
	}

	std::vector<VkVertexInputAttributeDescription> LveModel::Vertex::getAttributeDescriptions()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0; //correspond a la location précisée dans le vertex shader
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = offsetof(Vertex, position);

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1; //correspond a la location précisée dans le vertex shader
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributeDescriptions[1].offset = offsetof(Vertex, color);

		return attributeDescriptions;
	}

}