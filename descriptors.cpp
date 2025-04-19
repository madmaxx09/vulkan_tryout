#include "descriptors.hpp"
#include <iostream>

namespace wind
{
	VkDescriptorPool DescriptorPool::get_default_pool(EngineDevice &device)
	{
		VkDescriptorPool newPool;
		if (readyPools.size() != 0)
		{
			newPool = readyPools.back();
			readyPools.pop_back();
		}
		else
		{
			newPool = create_pool(device, maxSetsPerPool, ratios);

			maxSetsPerPool *= 1.5;
			if (maxSetsPerPool > 4092)
				maxSetsPerPool = 4092;
		}
		return newPool;
	}

	VkDescriptorPool DescriptorPool::get_pool(EngineDevice &device)
	{
		VkDescriptorPool newPool;
		if (readyPools.size() != 0)
		{
			newPool = readyPools.back();
			readyPools.pop_back();
		}
		else
		{
			newPool = create_pool(device, maxSetsPerPool, ratios);

			maxSetsPerPool *= 1.5;
			if (maxSetsPerPool > 4092)
				maxSetsPerPool = 4092;
		}
		return newPool;
	}

	VkDescriptorPool DescriptorPool::create_pool(EngineDevice &device, uint32_t setCount, std::vector<PoolSizeRatio> &poolRatios)
	{
		std::vector<VkDescriptorPoolSize> poolSizes;
		for (PoolSizeRatio ratio : poolRatios)
		{
			poolSizes.push_back(VkDescriptorPoolSize{
				.type = ratio.type,
				.descriptorCount = uint32_t(ratio.ratio * setCount)
			});
		}

		VkDescriptorPoolCreateInfo pool_info{};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.flags = 0;
		pool_info.maxSets = setCount;
		pool_info.poolSizeCount = (uint32_t)poolSizes.size();
		pool_info.pPoolSizes = poolSizes.data();

		VkDescriptorPool newPool;
		vkCreateDescriptorPool(device.device(), &pool_info, nullptr, &newPool);
		return newPool;
	}


	void DescriptorPool::init(EngineDevice &device, uint32_t maxSets, std::vector<PoolSizeRatio> &poolRatios)
	{
		ratios.clear();

		for (auto r : poolRatios)
		{
			ratios.push_back(r);
		}

		VkDescriptorPool newPool = create_pool(device, maxSets, poolRatios);

		maxSetsPerPool = maxSets * 1.5;

		readyPools.push_back(newPool);
	}

	void DescriptorPool::clear_pools(EngineDevice &device)
	{
		for (auto p : readyPools)
		{
			vkResetDescriptorPool(device.device(), p, 0);
		}
		for (auto p : fullPools)
		{
			vkResetDescriptorPool(device.device(), p, 0);
			readyPools.push_back(p);
		}
		fullPools.clear();
	}

	void DescriptorPool::destroy_pools(EngineDevice &device)
	{
		for (auto p : readyPools)
		{
			vkDestroyDescriptorPool(device.device(), p, nullptr);
		}
		readyPools.clear();

		for (auto p : fullPools)
		{
			vkDestroyDescriptorPool(device.device(), p, nullptr);
		}
		fullPools.clear();
	}

	void DescriptorPool::allocate(EngineDevice &device, VkDescriptorSetLayout layout, VkDescriptorSet &descriptorSet, void* pNext)
	{
		VkDescriptorPool pool_to_use = get_pool(device);
		
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.pNext = pNext;
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = pool_to_use;
		allocInfo.descriptorSetCount = 1; //this depends on use case ?
		allocInfo.pSetLayouts = &layout;

		if (vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet) == (VK_ERROR_OUT_OF_POOL_MEMORY || VK_ERROR_FRAGMENTED_POOL))
		{
			fullPools.push_back(pool_to_use);

			pool_to_use = get_pool(device);
			allocInfo.descriptorPool = pool_to_use;
			vkAllocateDescriptorSets(device.device(), &allocInfo, &descriptorSet);
		}

		readyPools.push_back(pool_to_use);
		//return descriptorSet;
	}

	void DescriptorWriter::write_buffer(int binding, VkBuffer buffer, size_t size, size_t offset, VkDescriptorType type, VkDescriptorBufferInfo &info)
	{
		info.buffer = buffer;
		info.offset = offset;
		info.range = size;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.dstSet = VK_NULL_HANDLE;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pBufferInfo = &info;

		writes.push_back(write);
	}

	void DescriptorWriter::write_image(int binding, VkImageView image, VkSampler sampler, VkImageLayout layout, VkDescriptorType type)
	{
		VkDescriptorImageInfo info{};
		info.sampler = sampler;
		info.imageView = image;
		info.imageLayout = layout;

		VkWriteDescriptorSet write{};
		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.dstBinding = binding;
		write.dstSet = VK_NULL_HANDLE;
		write.descriptorCount = 1;
		write.descriptorType = type;
		write.pImageInfo = &info;

		writes.push_back(write);
	}

	void DescriptorWriter::update_set(EngineDevice &device, VkDescriptorSet set)
	{
		for (VkWriteDescriptorSet &write : writes)
		{
			write.dstSet = set;
			//std::cout << "buffer : " << write.pBufferInfo->buffer << " set : " << set << std::endl;
		}

		vkUpdateDescriptorSets(device.device(), (uint32_t)writes.size(), writes.data(), 0, nullptr);
	}

	void DescriptorWriter::clear()
	{
		writes.clear();
	}

}