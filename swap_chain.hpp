#pragma once

#include "engine.hpp"

// vulkan headers
#include <vulkan/vulkan.h>

// std lib headers
#include <memory>
#include <string>
#include <vector>

namespace wind {
	
class LveSwapChain
{
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2; //number of frame that will be processed concurently, this affects multithreading handling and other things

		LveSwapChain(EngineDevice &deviceRef, VkExtent2D windowExtent);
		LveSwapChain(EngineDevice &deviceRef, VkExtent2D windowExtent, std::shared_ptr<LveSwapChain> previous);
		~LveSwapChain();

		LveSwapChain(const LveSwapChain &) = delete;
		LveSwapChain& operator=(const LveSwapChain &) = delete;

		VkFramebuffer getFrameBuffer(int index) { return swapChainFramebuffers[index]; }
		VkRenderPass getRenderPass() { return renderPass; }
		VkImageView getImageView(int index) { return swapChainImageViews[index]; }
		size_t imageCount() { return swapChainImages.size(); }
		VkFormat getSwapChainImageFormat() { return swapChainImageFormat; }
		VkExtent2D getSwapChainExtent() { return swapChainExtent; }
		uint32_t width() { return swapChainExtent.width; }
		uint32_t height() { return swapChainExtent.height; }

	float extentAspectRatio() {
		return static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height);
	}
	VkFormat findDepthFormat();

	VkResult acquireNextImage(uint32_t *imageIndex);
	VkResult submitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);

	bool compareSwapFormat(const LveSwapChain& swapChain) const
	{
	return swapChain.swapChainDepthFormat == swapChainDepthFormat &&
			swapChain.swapChainImageFormat == swapChainImageFormat;
	}

	private:
		void init();
		void createSwapChain();
		void createImageViews();
		void createDepthResources();
		void createRenderPass();
		void createFramebuffers();
		void createSyncObjects();

		// Helper functions
		VkSurfaceFormatKHR chooseSwapSurfaceFormat(
				const std::vector<VkSurfaceFormatKHR> &availableFormats);
		VkPresentModeKHR chooseSwapPresentMode(
				const std::vector<VkPresentModeKHR> &availablePresentModes);
		VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

		VkFormat swapChainImageFormat;
		VkFormat swapChainDepthFormat;
		VkExtent2D swapChainExtent;

		std::vector<VkFramebuffer> swapChainFramebuffers;
		VkRenderPass renderPass;

		std::vector<VkImage> depthImages;
		std::vector<VkDeviceMemory> depthImageMemorys;
		std::vector<VkImageView> depthImageViews;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;

		EngineDevice &device;
		VkExtent2D windowExtent;

		VkSwapchainKHR swapChain;
		std::shared_ptr<LveSwapChain> oldSwapchain;

		std::vector<VkSemaphore> imageAvailableSemaphores;//used to sync gpu queues and make sure that rendering is done before presenting the frame
		std::vector<VkSemaphore> renderFinishedSemaphores;
		std::vector<VkFence> inFlightFences;
		std::vector<VkFence> imagesInFlight;//vkFences act a bit like mutexes, but you can chose wether you wait or not
		size_t currentFrame = 0;
};

}
