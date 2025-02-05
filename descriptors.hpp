#pragma once
#include "engine.hpp"
#include <span>
#include <deque>

namespace wind
{
	//void createDescriptorsSetLayout(VkDescriptorSetLayout &layout);


	struct DescriptorWriter
	{
		std::vector<VkWriteDescriptorSet> writes;

		void write_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type);
		void write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type);

		void clear();
		void update_set(EngineDevice &device, VkDescriptorSet set);		
	};


	class DescriptorPool
	{
		public:
			struct PoolSizeRatio
			{
				VkDescriptorType type;
				float ratio;
			};
			void init(EngineDevice &device, uint32_t initialSets, std::vector<PoolSizeRatio> &poolRatios);
			void clear_pools(EngineDevice &device);
			void destroy_pools(EngineDevice &device);
			void allocate(EngineDevice &device, VkDescriptorSetLayout layout, VkDescriptorSet &descriptorSet, void* pNext);

		private:
			VkDescriptorPool get_pool(EngineDevice &device);
			VkDescriptorPool create_pool(EngineDevice &device, uint32_t setCount, std::vector<PoolSizeRatio> &poolRatios);

			std::vector<PoolSizeRatio> ratios;
			std::vector<VkDescriptorPool> fullPools;
			std::vector<VkDescriptorPool> readyPools;
			uint32_t maxSetsPerPool;
	};

}

