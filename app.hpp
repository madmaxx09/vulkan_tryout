#pragma once

#include "window.hpp"
#include "game_object.hpp"
#include "engine.hpp"
#include "renderer.hpp"
#include "descriptors.hpp"

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
			void LoadGameObjects();

			Window appWindow{WIDTH, HEIGHT, "wind"}; //initialises the window instance with GLFW
			EngineDevice device{appWindow};//sets up validation layer, bind glfw with our vkinstance and vksurfaceKHR finds the physical device, creates our logical device binds it with the command pool 
			LveRenderer lveRenderer{appWindow, device};
			
			DescriptorPool				globalDescriptorPool;//[LveSwapChain::MAX_FRAMES_IN_FLIGHT]; //a class that pre allocates some VkDescriptorPool 
			LveGameObject::Map	gameObjects;
	};
}