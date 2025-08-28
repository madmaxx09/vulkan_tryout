#include "app.hpp"
#include <array>
#include <iostream>
#include <chrono>
#include "simple_render_system.hpp"
#include "point_light_system.hpp"
#include "camera.hpp"
#include "keyboard.hpp"


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <unordered_map>

namespace wind
{
	App::App()
	{
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
		imGuiDescriptorPool.init(device, 1000, imGuiPoolRatios);
		initImGui();	
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
		Player player(viewerObject);

		KeyboardMovementController cameraController{};

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
			camera.setPerspectiveProjection(glm::radians(50.f), aspect, .1f, 50.f); //last 2 values are very relevant here cause objects outside these bounds will get clipped
			
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
				if (multiPlayer == 1) {
					client->Send(player);

					client->Recv();
				}
				memcpy(uboBuffers[frameIndex].data, &ubo, sizeof(GlobalUBO));


				//render phase ORDER MATTERS 
				lveRenderer.beginSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(frameInfo);
				
				pointLightSystem.render(frameInfo);
				RenderImgui(commandBuffer);

				//end frame
				//disabled vkFreeDescriptorSet in the imgui implFile seems sketchy need to investigate
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
		imGuiDescriptorPool.destroy_pools(device);
		vkDestroyDescriptorSetLayout(device.device(), layout, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		vkDestroyDescriptorPool(device.device(), infoImGui.DescriptorPool, nullptr);
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

		lveModel = LveModel::createModel_from_file(device, "obj_models/smooth_vase.obj");

		auto playerVase = LveGameObject::createGameObject();
		playerVase.model = lveModel;
		playerVase.transform.translation = {0.f, 0.3f, 0.5f};
		playerVase.transform.scale = 3.0f;
		playerVase.mass = 0.3f;
		gameObjects.emplace(playerVase.getId(), std::move(playerVase));

		

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

	void App::initImGui()
	{
		ImGui::CreateContext();
		ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui_ImplGlfw_InitForVulkan(appWindow.getGLFWwindow(), true);

		//ImGui_ImplVulkan_InitInfo info{};
		infoImGui.DescriptorPool = imGuiDescriptorPool.get_default_pool(device);
		infoImGui.Device = device.device();
		infoImGui.PhysicalDevice = device.getPhysicalDevice();
		infoImGui.Instance = device.getInstance();
		infoImGui.ImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
		infoImGui.MinImageCount = LveSwapChain::MAX_FRAMES_IN_FLIGHT;
		infoImGui.Queue = device.graphicsQueue();
		infoImGui.RenderPass = lveRenderer.getSwapChainRenderPass();
		ImGui_ImplVulkan_Init(&infoImGui);

		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void App::RenderImgui(const VkCommandBuffer &commandBuffer)
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
				
		static char inputBuffer[128] = "";

		if (ImGui::Begin("Menu"))
		{
			if (ImGui::BeginTabBar("Menu"))
			{
				if (ImGui::BeginTabItem("Connection"))
				{
					ImGui::TextWrapped("Enter the room code:");
					ImGui::InputText("##Input", inputBuffer, IM_ARRAYSIZE(inputBuffer));
			
					ImGui::SameLine();
			
					if (ImGui::Button("Connect")) {
						std::cout << "Connect clicked with input: " << inputBuffer << std::endl;
						std::string inputStr(inputBuffer);
						connectToServer(inputStr);
					}
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Add Items"))
				{
					if (ImGui::Button("Add vase"))
					{
						std::cout << "Add vase pressed" << std::endl;
						spawnVase();
					}
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
	
			// Champ texte
	
			ImVec2 windowSize = ImGui::GetWindowSize();
			ImVec2 textSize = ImGui::CalcTextSize("FPS: 000.0");
	
			// Position : coin inférieur droit moins la taille du texte et un petit padding
			ImVec2 pos(windowSize.x - textSize.x - 10.0f, windowSize.y - textSize.y - 10.0f);
	
			// Position relative au début de la fenêtre (curseur à 0,0)
			//ImGui::SetCursorPos(pos);
			ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
		}

		ImGui::End();

		ImGui::Render();

		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer, nullptr);		
	}

	void App::connectToServer(std::string &input)
	{
		std::cout << "in Connect" << std::endl;
		
		client = std::make_unique<Client>(input);
		multiPlayer = 1;
	}

	void App::spawnVase()
	{
		
	}
}