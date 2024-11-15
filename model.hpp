#pragma once

#include "engine.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <vector>


namespace wind 
{
	class LveModel
	{
		public:
			struct Vertex
			{
				glm::vec2	position;
				glm::vec3	color;
				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
			};

			LveModel(EngineDevice &device, const std::vector<Vertex> &vertices);
			~LveModel();
			
			LveModel(const LveModel & ) = delete;
			LveModel& operator=(const LveModel & ) = delete;
			
			void bind(VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);

		private:
			EngineDevice	&device;
			VkBuffer		vertexBuffer;
			VkDeviceMemory	vertexBufferMemory;
			uint32_t		vertexCount;

			void createVertexBuffers(const std::vector<Vertex> &vertices);
	};
}