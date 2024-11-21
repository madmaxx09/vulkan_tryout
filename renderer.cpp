#include "renderer.hpp"

#include <array>
#include <iostream>


namespace wind
{

	LveRenderer::LveRenderer(Window& window, EngineDevice& device) : appWindow{window}, device{device}
	{
		recreateSwapChain();
		CreateCommandBuffers();
	}

	LveRenderer::~LveRenderer()
	{
		FreeCommandBuffers();
	}
 
	void LveRenderer::CreateCommandBuffers()
	{
		//commandBuffers.resize(swapchain->imageCount());
		commandBuffers.resize(LveSwapChain::MAX_FRAMES_IN_FLIGHT);

		VkCommandBufferAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = device.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		if (vkAllocateCommandBuffers(device.device(), &allocInfo, commandBuffers.data()) != VK_SUCCESS)
			throw std::runtime_error("failed to allocate commande buffers");	
	}

	void LveRenderer::FreeCommandBuffers()
	{
		vkFreeCommandBuffers(device.device(), device.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());
		commandBuffers.clear();	
	}

	void LveRenderer::recreateSwapChain()
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
			std::shared_ptr<LveSwapChain> oldSwapchain = std::move(swapchain);
			swapchain = std::make_unique<LveSwapChain>(device, extent, oldSwapchain);

			if (!oldSwapchain->compareSwapFormat(*swapchain.get()))
			{
				throw std::runtime_error("Swap chain format has changed");
			}
		}
	}

	VkCommandBuffer LveRenderer::beginFrame()
	{
		assert(!isFrameStarted && "Can't call begin frame while in progress");
		auto result = swapchain->acquireNextImage(&currentImageIndex);
		//std::cout << imageIndex << std::endl;
		if (result == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}
		if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
			throw std::runtime_error("failed to acquire the next swapchain image");
		
		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to begin recording command buffer");
		}
		return commandBuffer;
	}

	void LveRenderer::endFrame()
	{
		assert(isFrameStarted && "Can't call end frame if not in progress");
		auto commandBuffer = getCurrentCommandBuffer();
		if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
			throw std::runtime_error("failed to record command buffer");
		auto result = swapchain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || appWindow.wasWindowResized())
		{
			appWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}
		else if (result != VK_SUCCESS)
			throw std::runtime_error("failed to present swap chain images");
		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % LveSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void LveRenderer::beginSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call beginSwapchain if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin renderpass on command buffer from a different frame");
		
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = swapchain->getRenderPass();
		renderPassInfo.framebuffer = swapchain->getFrameBuffer(currentImageIndex);
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = swapchain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
		clearValues[1].depthStencil = {1.0f, 0};
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(swapchain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(swapchain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		VkRect2D scissor{{0, 0}, swapchain->getSwapChainExtent()};

		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
		vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
	}

	void LveRenderer::endSwapchainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Can't call endSwapchain if frame is not in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Can't end renderpass on command buffer from a different frame");
		
		vkCmdEndRenderPass(commandBuffer);

	}
}