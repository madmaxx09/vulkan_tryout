#include "app.hpp"
#include <array>
#include <iostream>
#include <chrono>
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
#include "camera.hpp"
#include "keyboard.hpp"
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <unordered_map>

namespace wind
{
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

		std::vector<DescriptorPool::PoolSizeRatio> imGuiPoolRatios = {
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1 }
		};
		imGuiDescriptorPool.init(device, 1000, poolRatios);	
		LoadGameObjects();
	}

	App::~App()
	{
	}

	void App::run()
	{

		GlobalUBO ubo{};

		t_buffer uboBuffers[LveSwapChain::MAX_FRAMES_IN_FLIGHT];//one buffer per frame, to allow updating a buffer without affecting the currently rendered frame
		for (t_buffer &buffer : uboBuffers)
		{
			initialise_buffer(buffer, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
				device, sizeof(GlobalUBO));
			vkMapMemory(device.device(), buffer.memory, 0, sizeof(GlobalUBO), 0, &buffer.data);
		}

		//this whole block should look better
		VkDescriptorSetLayout layout{};

		std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};

		VkDescriptorSetLayoutBinding layoutBinding{};
		layoutBinding.binding = 0; //binding number in the shader
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS;
		layoutBinding.descriptorCount = 1;
		bindings[0] = layoutBinding;

		std::vector<VkDescriptorSetLayoutBinding> setBinding{};
		for (auto key_val : bindings)
		{
			setBinding.push_back(key_val.second);
		}

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(setBinding.size());
		layoutInfo.pBindings = setBinding.data();

		if (vkCreateDescriptorSetLayout(device.device(), &layoutInfo, nullptr, &layout) != VK_SUCCESS)
			throw std::runtime_error("failed to create descriptor set layout");
		//until here
		
		VkDescriptorSet globalDescriptorSets[LveSwapChain::MAX_FRAMES_IN_FLIGHT];
		for (int i = 0; i < LveSwapChain::MAX_FRAMES_IN_FLIGHT; i++)
		{
			DescriptorWriter writer{};
			VkDescriptorBufferInfo bufferInfo{};
			writer.write_buffer(0, uboBuffers[i].buffer, VK_WHOLE_SIZE, 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, bufferInfo);
			//need to write to buffer need buffer info (vk device size whole size and offset 0)
			globalDescriptorPool.allocate(device, layout, globalDescriptorSets[i], nullptr);
			writer.update_set(device, globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{device, lveRenderer.getSwapChainRenderPass(), layout}; //pipeline is created here
		for (auto &kv : gameObjects)
		{
			auto &obj = kv.second;

			if (obj.mass == EARTH)
				simpleRenderSystem.floor_y = obj.transform.translation.y;
		}
		PointLightSystem pointLightSystem{device, lveRenderer.getSwapChainRenderPass(), layout};
		LveCamera camera{};
		camera.setViewTarget(glm::vec3(-1.f, -2.f, 2.f), glm::vec3(0.f, 0.f, 2.5f));

		auto viewerObject = LveGameObject::createGameObject();
		viewerObject.transform.translation.z = -5.5f;


		KeyboardMovementController cameraController{};

		
		//imgui init //seperated render pass, command, buffer, pool could probly use my allocator
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(appWindow.getGLFWwindow(), true);

		ImGui_ImplVulkan_InitInfo info{};
		info.DescriptorPool = imGuiDescriptorPool.get_default_pool(device);
		info.Device = device.device();
		info.PhysicalDevice = device.getPhysicalDevice();
		info.Instance = device.getInstance();
		info.ImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
		info.MinImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
		//info.QueueFamily = divice.g
		info.Queue = device.graphicsQueue();
		info.RenderPass = lveRenderer.getSwapChainRenderPass();
		ImGui_ImplVulkan_Init(&info);

		//VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();
		ImGui_ImplVulkan_CreateFontsTexture();

		//imgui init ended , should put in other file/function

		auto currentTime = std::chrono::high_resolution_clock::now(); 
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
					camera,
					globalDescriptorSets[frameIndex],
					gameObjects
				};


				//update UBO /other buffers later maybe
				ubo.projection = camera.getProjection();
				ubo.view = camera.getView();
				ubo.inverseView = camera.getInverseViewMatrix();
				pointLightSystem.update(frameInfo, ubo);
				
				//selfexplanatory
				simpleRenderSystem.applyPhysics(frameInfo, ubo);

				memcpy(uboBuffers[frameIndex].data, &ubo, sizeof(GlobalUBO));


				//render phase ORDER MATTERS 
				lveRenderer.beginSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo);
				
				pointLightSystem.render(frameInfo);

				ImGui_ImplVulkan_NewFrame();
				ImGui_ImplGlfw_NewFrame();
				ImGui::NewFrame();

				ImGui::ShowDemoWindow();

				ImGui::Render();

				ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, nullptr);

				//end frame
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
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	

	void App::LoadGameObjects()
	{
		std::shared_ptr<LveModel> lveModel = LveModel::createModel_from_file(device, "obj_models/flat_vase.obj");

		auto flatVase = LveGameObject::createGameObject();
		flatVase.model = lveModel;
		flatVase.transform.translation = {0.5f, 0.3f, 0.f};
		flatVase.transform.scale = 3.0f;
		flatVase.mass = 0.3f;
		gameObjects.emplace(flatVase.getId(), std::move(flatVase));

		lveModel = LveModel::createModel_from_file(device, "obj_models/smooth_vase.obj");

		auto smoothVase = LveGameObject::createGameObject();
		smoothVase.model = lveModel;
		smoothVase.transform.translation = {-0.5f, -2.5f, 0.f};
		smoothVase.transform.scale = 3.0f;
		smoothVase.mass = 0.3f;
		gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

		lveModel = LveModel::createModel_from_file(device, "obj_models/floor.obj");

		auto floor = LveGameObject::createGameObject();
		floor.model = lveModel;
		floor.transform.translation = {0.f, 0.5f, 0.f};
		floor.transform.scale = 3.0f;
		floor.mass = EARTH;
		gameObjects.emplace(floor.getId(), std::move(floor));
		
		std::vector<glm::vec3> lightColors {
			{1.f, .1f, .1f},
			{.1f, .1f, 1.f},
			{.1f, 1.f, .1f},
			{1.f, 1.f, .1f},
			{.1f, 1.f, 1.f},
			{1.f, 1.f, 1.f}
		};

		for (int i = 0; i < lightColors.size(); i++)
		{
			auto pointLight = LveGameObject::create_point_light(0.2f);
			pointLight.color = lightColors[i];
			auto rotateLight = glm::rotate(
				glm::mat4(1.f),
				(i * glm::two_pi<float>()) / lightColors.size(),
				{0.f, -1.f, 0.f});
			pointLight.transform.translation = glm::vec3(rotateLight * glm::vec4(-1.f, -1.f, -1.f, 1.f));
			gameObjects.emplace(pointLight.getId(), std::move(pointLight));
		}


		// std::shared_ptr<LveModel> lveModel = LveModel::createModel_from_file(device, "obj_models/viking_room.obj");

		// auto viking = LveGameObject::createGameObject();
		// viking.model = lveModel;
		// viking.transform.translation = {0.f, 0.5f, 0.f};
		// viking.transform.scale = 3.0f;
		// gameObjects.emplace(viking.getId(), std::move(viking));
	}
}