#include "app.hpp"
#include <array>
#include <iostream>

namespace wind
{
	App::App()
	{
		LoadModels();
		CreatePipelineLayout();
		recreateSwapChain();
		CreateCommandBuffers();
	}

	App::~App()
	{
		vkDestroyPipelineLayout(device.device(), pipelineLayout, nullptr);
	}

	void App::run()
	{
		while(!appWindow.shouldClose())
		{
			glfwPollEvents(); //get events like keystrokes but in our case mostly clicking x button
			drawFrame();
		}
		vkDeviceWaitIdle(device.device());
	}

	void App::CreatePipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};

		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		if (vkCreatePipelineLayout(device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS)
			throw std::runtime_error("failed to create pipeline layout");		
	}

	void App::CreatePipeline()
	{
		PipelineConfigInfo pipelineConfig{};
		Pipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = swapchain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		pipeline = std::make_unique<Pipeline>(
			device,
			"shaders/shader.vert.spv",
			"shaders/frag.frag.spv",
			pipelineConfig);
	}

	void App::CreateCommandBuffers()
	{
		commandBuffers.resize(swapchain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate commande buffers");	
	}

	void App::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();	
	}

	void App::recordCommandBuffer(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapchain->getRenderPass();
		renderPassInfo.framebuffer = swapchain->getFrameBuffer(imageIndex);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, swapchain->getSwapChainExtent()};

		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

		pipeline->bind(commandBuffers[imageIndex]);
		model->bind(commandBuffers[imageIndex]);
		model->draw(commandBuffers[imageIndex]);

		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer");
	}

	void App::recreateSwapChain()
	{
		auto extent = appWindow.getExtent();
		while(extent.width == 0 || extent.height == 0)
		{
			extent = appWindow.getExtent();
			glfwWaitEvents();
		}
		vkDeviceWaitIdle(device.device());

		if (swapchain == nullptr)
			swapchain = std::make_unique<LveSwapChain>(device, extent);
		else {
			swapchain = std::make_unique<LveSwapChain>(device, extent, std::move(swapchain));
			if (swapchain->imageCount() != commandBuffers.size())
				FreeCommandBuffers();
				CreateCommandBuffers();
		}
		CreatePipeline();
	}

	void App::LoadModels()
	{
		std::vector<LveModel::Vertex> vertices {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		model = std::make_unique<LveModel>(device, vertices);
	}

	void App::drawFrame()
	{
		uint32_t imageIndex;
		auto result = swapchain->acquireNextImage(&imageIndex);
		//std::cout << imageIndex << std::endl;
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire the next swapchain image");
		
		recordCommandBuffer(imageIndex);
		result = swapchain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || appWindow.wasWindowResized())
		{
			appWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (result != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain images");
	}
}