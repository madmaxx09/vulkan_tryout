#include "pipeline.hpp"
#include "model.hpp"

#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cassert>

namespace wind
{
	Pipeline::Pipeline(EngineDevice& device, const std::string & vertFilePath, const std::string & fragFilePath, const PipelineConfigInfo& configInfo) : device{device}
	{
		createGraphicsPipeline(vertFilePath, fragFilePath, configInfo);
	}

	Pipeline::~Pipeline()
	{
		vkDestroyShaderModule(device.device(), vertexShaderModule, nullptr);
		vkDestroyShaderModule(device.device(), fragShaderModule, nullptr);
		vkDestroyPipeline(device.device(), graphicsPipeline,  nullptr);	
	}

	std::vector<char> Pipeline::readFile(const std::string& filePath)
	{
		std::ifstream file(filePath, std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file " + filePath);
		}
		size_t fileSize = static_cast<size_t>(file.tellg());
		std::vector<char> buffer(fileSize);

		file.seekg(0);
		file.read(buffer.data(), fileSize); //errors should be checked here // we use vector.data() because ifstream.read() take a pointer to an array where to store what is read
		file.close();

		return buffer;
	}

	void Pipeline::createGraphicsPipeline(const std::string & vertFilePath, const std::string & fragFilePath, const PipelineConfigInfo& configInfo)
	{
		assert(configInfo.pipelineLayout != VK_NULL_HANDLE && "Cant create the pipeline, no layout provided in configInfo");
		assert(configInfo.renderPass != VK_NULL_HANDLE && "Cant create the pipeline, no renderpass provided in configInfo");
		auto vertCode = readFile(vertFilePath);
		auto fragCode = readFile(fragFilePath);

		createShaderModule(vertCode, &vertexShaderModule);
		createShaderModule(fragCode, &fragShaderModule);

		VkPipelineShaderStageCreateInfo shaderStage[2];
		//vertex shader
		shaderStage[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
		shaderStage[0].module = vertexShaderModule;
		shaderStage[0].pName = "main"; //name of the entry function in the vertex shader
		shaderStage[0].flags = 0;
		shaderStage[0].pNext = nullptr;
		shaderStage[0].pSpecializationInfo = nullptr; //usefull to customize shader functionnality
		//fragment shader
		shaderStage[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		shaderStage[1].module = fragShaderModule;
		shaderStage[1].pName = "main"; //name of the entry function in the vertex shader
		shaderStage[1].flags = 0;
		shaderStage[1].pNext = nullptr;
		shaderStage[1].pSpecializationInfo = nullptr; //usefull to customize shader functionnality

		auto bindingDescriptions = LveModel::Vertex::getBindingDescriptions();
		auto attributeDescriptions = LveModel::Vertex::getAttributeDescriptions();
		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
		vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
		vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();
		vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();

		//je rassemble ici toutes les structures contenant mes infos 
		VkGraphicsPipelineCreateInfo pipelineInfo{};
		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.stageCount = 2; //nombre d'étape programmables dans le pipeline , pour l'instant les shaders vertex et frag
		pipelineInfo.pStages = shaderStage;
		pipelineInfo.pVertexInputState = &vertexInputInfo;
		pipelineInfo.pInputAssemblyState = &configInfo.inputAssemblyInfo;
		pipelineInfo.pViewportState = &configInfo.viewportInfo;
		pipelineInfo.pRasterizationState = &configInfo.rasterizationInfo;
		pipelineInfo.pColorBlendState = &configInfo.colorBlendInfo;
		pipelineInfo.pDepthStencilState = &configInfo.depthStencilInfo;
		pipelineInfo.pMultisampleState = &configInfo.multisampleInfo;
		pipelineInfo.pDynamicState = &configInfo.dynamicStateInfo;

		pipelineInfo.layout = configInfo.pipelineLayout;
		pipelineInfo.renderPass = configInfo.renderPass;
		pipelineInfo.subpass = configInfo.subpass;

		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

		if (vkCreateGraphicsPipelines(device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create the graphics pipeline");
		}
	}

	void Pipeline::createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		if (vkCreateShaderModule(device.device(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to created shader module");
		}
	}

	void Pipeline::defaultPipelineConfigInfo(PipelineConfigInfo& configInfo)
	{
		configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; //
		configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST; //on précise ce que la liste de sommets qui rentre dans le pipeline représente, (ex 3 points peuvent être un ligne ou un triangle)
		configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE; //si on utilise une topologie qui réutilise les sommets précedents pour le triangle suivant cela permet de mettre cette logique en pause sinon tout nos triangles seraient obligatoirement connectés

		configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		configInfo.viewportInfo.viewportCount = 1;
		configInfo.viewportInfo.pViewports = nullptr;
		configInfo.viewportInfo.scissorCount = 1;
		configInfo.viewportInfo.pScissors = nullptr;

		configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		configInfo.rasterizationInfo.depthClampEnable = VK_FALSE; //force the z component to be between 0and 1 if less than 0 if more than 1
		configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE; 
		configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL; //filling the triangle
		configInfo.rasterizationInfo.lineWidth = 1.0f;
		configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_NONE; //très important !!! quand on active ce paramètre on décide ne pas afficher les triangles dont on voit le "dos"
		configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_CLOCKWISE; //comment savoir de quel coté on est ? grâçe à ce paramètre qui nous dis que la face visible est celle dont les sommets sont obtenus dans le sens des aiguilles d'une montre
		configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE; 
		configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f;
		configInfo.rasterizationInfo.depthBiasClamp = 0.0f;
		configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f; //last 3 are optionnal

		configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; //multisampling , (comme j'ai fait dans mon raytracer)
		configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE; //cela permet de réduire l'aliasing en bordure de fragments en prenant plusieurs sample de pixels sans prendre le centre à chaque fois
		configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		configInfo.multisampleInfo.minSampleShading = 1.0f;
		configInfo.multisampleInfo.pSampleMask = nullptr;
		configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
		configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE; // last 4 are optionnal

		configInfo.colorBlendAttachment.colorWriteMask =
			VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
			VK_COLOR_COMPONENT_A_BIT;
		configInfo.colorBlendAttachment.blendEnable = VK_FALSE; 
		configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
		configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD; //last 6 are optionnal

		configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
		configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; //optionnal
		configInfo.colorBlendInfo.attachmentCount = 1;
		configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
		configInfo.colorBlendInfo.blendConstants[0] = 0.0f; //optionnal
		configInfo.colorBlendInfo.blendConstants[1] = 0.0f; //optionnal
		configInfo.colorBlendInfo.blendConstants[2] = 0.0f; //optionnal
		configInfo.colorBlendInfo.blendConstants[3] = 0.0f; //optionnal

		configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		configInfo.depthStencilInfo.depthTestEnable = VK_TRUE; //ces valeurs servent à savoir si un fragment est devant ou derrière un autre dans le champ 
		configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;//en fonction celui si sera discard ou utilisé
		configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
		configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.minDepthBounds = 0.0f;
		configInfo.depthStencilInfo.maxDepthBounds = 1.0f;
		configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
		configInfo.depthStencilInfo.front = {};
		configInfo.depthStencilInfo.back = {};

		configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
		configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
		configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
		configInfo.dynamicStateInfo.flags = 0;
	}

	void Pipeline::bind(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);

	}
}