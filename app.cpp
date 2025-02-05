#include "app.hpp"
#include <array>
#include <iostream>
#include <chrono>
#include "simple_render_system.hpp"
#include "camera.hpp"
#include "keyboard.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace wind
{

	struct GlobalUBO
	{
		glm::mat4	projectionView{1.f};
		glm::vec3	lightDirection = glm::normalize(glm::vec3{1.f, -3.f, -1.f});
	};

	App::App()
	{
		// std::cout << "Size of app : " << sizeof(App) << std::endl;
		// std::cout << "Size of window : " << sizeof(Window) << std::endl;
		// std::cout << "Size of cam : " << sizeof(LveCamera) << std::endl;
		// std::cout << "Size of engine : " << sizeof(EngineDevice) << std::endl;
		// std::cout << "Size of renderer : " << sizeof(LveRenderer) << std::endl;
		// for (int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		// {
		std::vector<DescriptorPool::PoolSizeRatio> poolRatios = {
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 3},
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3},
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 3},
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4}
		};
		globalDescriptorPool.init(device, 100, poolRatios);
		//}
		LoadGameObjects();
	}

	App::~App()
	{
	}

	void App::run()
	{
		SimpleRenderSystem simpleRenderSystem{device, lveRenderer.getSwapChainRenderPass()};
		LveCamera camera{};
		camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		auto viewerObject = LveGameObject::createGameObject();
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now(); 

		GlobalUBO ubo{};

		t_buffer uboBuffers[LveSwapChain::MAX_FRAMES_IN_FLIGHT];//one buffer per frame, to allow updating a buffer without affecting the currently rendered frame
		for (t_buffer &buffer : uboBuffers)
		{
			initialise_buffer(buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				device, sizeof(GlobalUBO));
			vkMapMemory(device.device(), buffer.memory, 0, sizeof(GlobalUBO), 0, &buffer.data);
			std::cout << "does buffer == buffer : " << buffer.buffer << std::endl;

		}
		std::cout << "Random buffer data : " << uboBuffers[0].buffer << std::endl;
		//this whole block should look better
		VkDescriptorSetLayout layout{};

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0; //binding number in the shader
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		layoutBinding.descriptorCount = 1;
		layoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &layoutBinding;
		if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &layout) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout");
		//until here
		

		VkDescriptorSet globalDescriptorSets[LveSwapChain::MAX_FRAMES_IN_FLIGHT];
		for (int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		{
			DescriptorWriter writer{};
			std::cout << "does buffer == buffer : " << uboBuffers[i].buffer << std::endl;

			writer.write_buffer(0, uboBuffers[i].buffer, VK_WHOLE_SIZE, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER);
			//need to write to buffer need buffer info (vk device size whole size and offset 0)
			globalDescriptorPool.allocate(device, layout, globalDescriptorSets[i], nullptr);
			writer.update_set(device, globalDescriptorSets[i]);
		}


		while(!appWindow.shouldClose())
		{
			glfwPollEvents(); //get events like keystrokes/clicking/...

			auto newTime = std::chrono::high_resolution_clock::now(); 
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
			currentTime = newTime;

			cameraController.moveInPlaneXZ(appWindow.getGLFWwindow(), frameTime, viewerObject);
			camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);

			float aspect = lveRenderer.getAspectRatio();
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 10.f); //last 2 values are very relevant here cause objects outside these bounds will get clipped
			
			if (auto commandBuffer = lveRenderer.beginFrame())
			{
				int frameIndex = lveRenderer.getFrameIndex();
				s_frame_info frameInfo{
					frameIndex,
					frameTime, 
					commandBuffer,
					camera
				};


				//update UBO /other buffers later maybe
				ubo.projectionView = camera.getProjection() * camera.getView();
				memcpy(uboBuffers[frameIndex].data, &ubo, sizeof(GlobalUBO));


				//render phase
				lveRenderer.beginSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
				lveRenderer.endSwapchainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(device.device());


		//memory cleanup
		for (t_buffer &buffer : uboBuffers)
		{
			destroy_buffer(buffer, device);
		}
		globalDescriptorPool.destroy_pools(device);
		vkDestroyDescriptorSetLayout(device.device(), layout, nullptr);
	}

	

	void App::LoadGameObjects()
	{
		std::shared_ptr<LveModel> lveModel = LveModel::createModel_from_file(device, "obj_models/flat_vase.obj");

		auto flatVase = LveGameObject::createGameObject();
		flatVase.model = lveModel;
		flatVase.transform.translation = {0.5f, 0.5f, 2.5f};
		flatVase.transform.scale = 3.0f;
		gameObjects.push_back(std::move(flatVase));

		lveModel = LveModel::createModel_from_file(device, "obj_models/smooth_vase.obj");

		auto gameObj = LveGameObject::createGameObject();
		gameObj.model = lveModel;
		gameObj.transform.translation = {-0.5f, 0.5f, 2.5f};
		gameObj.transform.scale = 3.0f;
		gameObjects.push_back(std::move(gameObj));
	}
}