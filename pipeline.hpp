#pragma once
#include <string>
#include <vector>
#include "engine.hpp"

namespace wind
{
	struct PipelineConfigInfo
	{
		PipelineConfigInfo(const PipelineConfigInfo&) = delete;
		PipelineConfigInfo& operator=(const PipelineConfigInfo) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateInfo;

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
			static void defaultPipelineConfigInfo(PipelineConfigInfo& configInfo);
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