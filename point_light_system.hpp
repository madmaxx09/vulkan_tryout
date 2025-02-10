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
	class PointLightSystem
	{
		public:
			PointLightSystem(EngineDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
			~PointLightSystem();

			PointLightSystem(const PointLightSystem & ) = delete;
			PointLightSystem& operator=(const PointLightSystem & ) = delete;

			void render(s_frame_info &frameinfo);


		private:
			void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
			void CreatePipeline(VkRenderPass renderPass);
			
			EngineDevice& device;

			std::unique_ptr<Pipeline> pipeline; //probly stack allocatable
			VkPipelineLayout pipelineLayout;
	};
}