#pragma once

#include "pipeline.hpp"
#include "game_object.hpp"
#include "engine.hpp"
#include "camera.hpp"
#include "frame_info.hpp"

#include <memory>
#include <vector>
#include <stdexcept>

namespace wind
{
	class SimpleRenderSystem
	{
		public:
			SimpleRenderSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
			~SimpleRenderSystem();

			SimpleRenderSystem(const SimpleRenderSystem & ) = delete;
			SimpleRenderSystem& operator=(const SimpleRenderSystem & ) = delete;

			void applyPhysics(s_frame_info &frameinfo, GlobalUBO &ubo);
			void renderGameObjects(s_frame_info &frameinfo);
			float floor_y;


		private:
			void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
			void CreatePipeline(VkRenderPass renderPass);
			
			EngineDevice& device;

			std::unique_ptr<Pipeline> pipeline; //probly stack allocatable
			VkPipelineLayout pipelineLayout;

			//physical properties

	};
}