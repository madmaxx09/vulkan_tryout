#pragma once

#include "window.hpp"
#include "engine.hpp"
#include "swap_chain.hpp"

#include <memory>
#include <vector>
#include <stdexcept>
#include <cassert>

namespace wind
{
	class LveRenderer
	{
		public:
			LveRenderer(Window& window, EngineDevice& device);
			~LveRenderer();

			LveRenderer(const LveRenderer & ) = delete;
			LveRenderer& operator=(const LveRenderer & ) = delete;

			VkCommandBuffer beginFrame();
			void endFrame();
			void beginSwapchainRenderPass(VkCommandBuffer commandBuffer);
			void endSwapchainRenderPass(VkCommandBuffer commandBuffer);

			VkRenderPass getSwapChainRenderPass() const { return swapchain->getRenderPass(); }
			bool isFrameInProgress() const { return(isFrameStarted); }
			VkCommandBuffer getCurrentCommandBuffer() const {
				assert(isFrameStarted && "Can't get command buffer is frame is not in progress");
				return commandBuffers[currentFrameIndex];
			}

			int getFrameIndex() const { 
				assert(isFrameStarted && "Can't get frame index when frame not in prog");
				return currentFrameIndex;
			}

		private:
			void CreateCommandBuffers();
			void FreeCommandBuffers();
			void recreateSwapChain();

			Window& appWindow;
			EngineDevice& device;
			std::unique_ptr<LveSwapChain> swapchain;
			std::vector<VkCommandBuffer> commandBuffers;

			uint32_t currentImageIndex;
			int	currentFrameIndex;
			bool isFrameStarted = false;

			// Pipeline pipeline{
			// 	device,
			// 	"shaders/shader.vert.spv",
			// 	"shaders/frag.frag.spv", 
			// 	Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
	};
}