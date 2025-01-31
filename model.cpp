#include "model.hpp"
#include "utils.hpp"

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include <unordered_map>
#include <cstring>
#include <cassert>
#include <iostream>


namespace std
{
	template <>
	struct hash<wind::LveModel::Vertex>
	{
		size_t operator()(wind::LveModel::Vertex const &vertex) const
		{
			size_t seed = 0;
			wind::hashCombine(seed, vertex.position, vertex.color, vertex.normal, vertex.uv);
			return seed;
		}
	};
}

namespace wind
{
	LveModel::LveModel(EngineDevice &device, const LveModel::Builder &builder) : device{device}
	{
		createVertexBuffers(builder.vertices);
		createIndexBuffers(builder.indices);
	}

	LveModel::~LveModel()
	{
		vkDestroyBuffer(device.device(), vertexBuffer, nullptr);
		vkFreeMemory(device.device(), vertexBufferMemory, nullptr);
		if (hasIndexBuffer)
		{
			vkDestroyBuffer(device.device(), indexBuffer, nullptr);
			vkFreeMemory(device.device(), indexBufferMemory, nullptr);
		}
	}


	std::unique_ptr<LveModel> LveModel::createModel_from_file(EngineDevice &device, const std::string &filepath)
	{
		Builder builder{};

		builder.loadModel(filepath);
		std::cout << "Vertices : " << builder.vertices.size() << std::endl;
		return std::make_unique<LveModel>(device, builder);
	}

	void LveModel::bind(VkCommandBuffer commandBuffer)
	{
		VkBuffer	buffers[] = {vertexBuffer};
		VkDeviceSize	offsets[] = {0};
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		if (hasIndexBuffer)
			vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
	}

	void LveModel::draw(VkCommandBuffer commandBuffer)
	{
		if (hasIndexBuffer)
			vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
		else
			vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
	}

	void LveModel::createVertexBuffers(const std::vector<Vertex> &vertices)
	{
		vertexCount = static_cast<uint32_t>(vertices.size());
		assert(vertexCount >= 3 && "Vertex count should be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

		VkBuffer stagingBuffer;//using staging buffer is relevant when using static meshes, but if the cpu constantly applies variations to that mesh, the performance gain will be neglected or even worse
		VkDeviceMemory stagingBufferMemory; //staging buffer is needed to then copy it into the gpu local vertex buffer
		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);
		
		void *data;
		vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data); //map une partie de la mémoire du Cpu pour matcher la mémoire du gpu dans enginedevice
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.device(), stagingBufferMemory);

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, //precise que notre buffer contiendra des infos de vertex
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, //buffer will be local to the gpu, reducing overhead, we loose cpu view of the buffer to achieve that
			vertexBuffer,
			vertexBufferMemory);

		device.copyBuffer(stagingBuffer, vertexBuffer, bufferSize); 
		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
	}

	void LveModel::createIndexBuffers(const std::vector<u_int32_t> &indices)
	{
		indexCount = static_cast<uint32_t>(indices.size());
		hasIndexBuffer = indexCount > 0;
		if (!hasIndexBuffer)
			return;

		VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingBufferMemory;
		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			stagingBuffer,
			stagingBufferMemory);
		
		void *data;
		vkMapMemory(device.device(), stagingBufferMemory, 0, bufferSize, 0, &data); //map une partie de la mémoire du Cpu pour matcher la mémoire du gpu dans enginedevice
		memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(device.device(), stagingBufferMemory);

		device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, //precise que notre buffer contiendra des infos de vertex
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, 
			indexBuffer,
			indexBufferMemory);

		device.copyBuffer(stagingBuffer, indexBuffer, bufferSize);
		vkDestroyBuffer(device.device(), stagingBuffer, nullptr);
		vkFreeMemory(device.device(), stagingBufferMemory, nullptr);
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
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};

		attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});//first arg is the specified location in our vertex shader
		attributeDescriptions.push_back({1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)}); //offset is crucial
		attributeDescriptions.push_back({2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});
		attributeDescriptions.push_back({3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

		return attributeDescriptions;
	}

	void LveModel::Builder::loadModel(const std::string &filepath)
	{
		tinyobj::attrib_t attrib; //stores textures coord, position, color, normal
		std::vector<tinyobj::shape_t> shapes;//index values
		std::vector<tinyobj::material_t> materials;

		std::string warn, error;

		if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error, filepath.c_str()))
			throw std::runtime_error(warn + error);
		vertices.clear();
		indices.clear();

		std::unordered_map<Vertex, u_int32_t> unique_vertices{};
		for (const auto &shape : shapes)
		{
			for (const auto &mesh_index : shape.mesh.indices)
			{
				Vertex vertex{};
				if (mesh_index.vertex_index >= 0) //if value is negative then no index was provided
				{
					vertex.position = {
						attrib.vertices[3 * mesh_index.vertex_index + 0],
						attrib.vertices[3 * mesh_index.vertex_index + 1],
						attrib.vertices[3 * mesh_index.vertex_index + 2]
					};

					vertex.color = {
						attrib.colors[3 * mesh_index.vertex_index + 0],
						attrib.colors[3 * mesh_index.vertex_index + 1],
						attrib.colors[3 * mesh_index.vertex_index + 2]
					};

				}
				if (mesh_index.normal_index >= 0) //if value is negative then no index was provided
				{
					vertex.normal = {
						attrib.normals[3 * mesh_index.normal_index + 0],
						attrib.normals[3 * mesh_index.normal_index + 1],
						attrib.normals[3 * mesh_index.normal_index + 2]
					};
				}
				if (mesh_index.texcoord_index >= 0) //if value is negative then no index was provided
				{
					vertex.uv = {
						attrib.vertices[3 * mesh_index.texcoord_index + 0],
						attrib.vertices[3 * mesh_index.texcoord_index + 1]
					};
				}
				if (unique_vertices.count(vertex) == 0) //check if vertex is new
				{
					unique_vertices[vertex] = static_cast<u_int32_t>(vertices.size());//to remember his position 
					vertices.push_back(vertex);
				}
				indices.push_back(unique_vertices[vertex]); //give the right position to the indices vector
			}
		}
	}
}