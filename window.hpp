#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <string>
#include <stdexcept>


namespace wind 
{
	class Window
	{
		private:
			GLFWwindow *window;
			int _width;
			int _height;
			bool frameBufferResized = false;

			std::string window_name;

			void init_window();
			static void frameBufferResizedCallback(GLFWwindow* window, int width, int height);
		
		public:
			Window(int width, int height, std::string name);
			~Window();

			Window(const Window &) = delete; //ces deux fonctions servent à désactiver le constructeur de copie 
			Window &operator=(const Window &) = delete; // et l'opérateur d'assignations car on ne veut pas dupliquer notre classe par mégarde étant donné qu'une de ses variables est un pointeur

			VkExtent2D getExtent() { return {static_cast<uint32_t>(_width), static_cast<uint32_t>(_height)}; }
			bool shouldClose() { return glfwWindowShouldClose(window);}
			void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
			bool wasWindowResized() { return frameBufferResized; }
			void resetWindowResizedFlag() { frameBufferResized = false; }
	};
}