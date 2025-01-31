#pragma once

#include "engine.hpp"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "initialise_buffers.hpp"


namespace wind 
{
	class LveModel
	{
		public:
			struct Vertex
			{
				glm::vec3	position{};
				glm::vec3	color{};
				glm::vec3	normal{};
				glm::vec2	uv{};


				static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
				static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

				bool operator==(const Vertex &other) const
				{
					return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
				}
			};

			struct Builder
			{
				std::vector<Vertex> vertices{}; //to build/link our vertex buffer and index buffer
				std::vector<uint32_t> indices{};

				void loadModel (const std::string & filepath);
			};

			LveModel(EngineDevice &device, const LveModel::Builder &builder);
			~LveModel();
			
			LveModel(const LveModel & ) = delete;
			LveModel& operator=(const LveModel & ) = delete;


			static std::unique_ptr<LveModel> createModel_from_file(EngineDevice &device, const std::string &filepath);

			void bind(VkCommandBuffer commandBuffer);
			void draw(VkCommandBuffer commandBuffer);

		private:
			EngineDevice	&device;

			// VkBuffer		vertexBuffer;
			// VkDeviceMemory	vertexBufferMemory;
			t_buffer 		vertexBuffer;
			uint32_t		vertexCount;

			// VkBuffer		indexBuffer;
			// VkDeviceMemory	indexBufferMemory;
			t_buffer		indexBuffer;
			uint32_t		indexCount;
			bool			hasIndexBuffer = false;

			void createVertexBuffers(const std::vector<Vertex> &vertices);
			void createIndexBuffers(const std::vector<u_int32_t> &indices);
	};
}