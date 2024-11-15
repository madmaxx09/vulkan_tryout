#pragma once
#include <string>
#include <vector>
#include "engine.hpp"

namespace wind
{
	struct PipelineConfigInfo
	{
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};
	class Pipeline
	{
		public:
			Pipeline(
				EngineDevice& device,
				const std::string & vertFilePath,
				const std::string& fragFilePath,
				const PipelineConfigInfo& configInfo);
			~Pipeline();

			Pipeline(const Pipeline&) = delete;
			Pipeline& operator=(const Pipeline&) = delete;

			void bind(VkCommandBuffer commandBuffer); 
			static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
		private:
			static std::vector<char> readFile(const std::string& filePath);

			void createGraphicsPipeline(
				const std::string & vertFilePath,
				const std::string & fragFilePath,
				const PipelineConfigInfo& configInfo);

			void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);
			
			EngineDevice& device;
			VkPipeline graphicsPipeline;
			VkShaderModule vertexShaderModule;
			VkShaderModule fragShaderModule;
		
	};
}