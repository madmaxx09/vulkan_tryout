#pragma once

#include "window.hpp"
#include "game_object.hpp"
#include "engine.hpp"
#include "renderer.hpp"
#include "client.hpp"
#include "player.hpp"
#include "descriptors.hpp"
#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

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
			void RenderImgui(const VkCommandBuffer &commandBuffer);

			private:

			void LoadGameObjects();
			void connectToServer(std::string &input);
			void initImGui();
			void spawnVase();

			Window appWindow{WIDTH, HEIGHT, "wind"}; //initialises the window instance with GLFW
			EngineDevice device{appWindow};//sets up validation layer, bind glfw with our vkinstance and vksurfaceKHR finds the physical device, creates our logical device binds it with the command pool 
			LveRenderer lveRenderer{appWindow, device};
			std::unique_ptr<Client> client = nullptr;
			
			DescriptorPool				globalDescriptorPool;//[LveSwapChain::MAX_FRAMES_IN_FLIGHT]; //a class that pre allocates some VkDescriptorPool 
			DescriptorPool				imGuiDescriptorPool;
			ImGui_ImplVulkan_InitInfo	infoImGui{};

			LveGameObject::Map	gameObjects;
			bool multiPlayer = false;
	};
}