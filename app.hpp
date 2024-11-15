#pragma once

#include "window.hpp"
#include "pipeline.hpp"
#include "engine.hpp"
#include "swap_chain.hpp"
#include "model.hpp"

#include <memory>
#include <vector>
#include <stdexcept>

namespace wind
{
	class App
	{
		public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;
			App();
			~App();

			App(const App & ) = delete;
			App& operator=(const App & ) = delete;
			void run();
		private:
			void LoadModels();
			void CreatePipelineLayout();
			void CreatePipeline();
			void CreateCommandBuffers();
			void drawFrame();
			void recreateSwapChain();
			void recordCommandBuffer(int imageIndex);

			Window appWindow{WIDTH, HEIGHT, "wind"};
			EngineDevice device{appWindow};
			std::unique_ptr<LveSwapChain> swapchain;
			std::unique_ptr<Pipeline> pipeline;
			VkPipelineLayout pipelineLayout;
			std::vector<VkCommandBuffer> commandBuffers;
			std::unique_ptr<LveModel>	model;

			// Pipeline pipeline{
			// 	device,
			// 	"shaders/shader.vert.spv",
			// 	"shaders/frag.frag.spv", 
			// 	Pipeline::defaultPipelineConfigInfo(WIDTH, HEIGHT)};
	};
}