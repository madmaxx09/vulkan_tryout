#include "window.hpp"

namespace wind
{
	Window::Window(int width, int height, std::string name) : _width(width), _height(height), window_name(name)
	{
		init_window();
	}

	void Window::init_window()
	{
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API); //pour empécher que le fenetre s'ouvre avec son contexte par défaut qui est opengl
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE); //pour empecher le resize car celui ci sera gérer autrement

		window = glfwCreateWindow(_width, _height, window_name.c_str(), nullptr, nullptr); //param 3 doit etre cstr, 4 param pour etre fullscreen, 5 utile si on a une fenetre avec un contexte opengl
		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, frameBufferResizedCallback);
	}
	Window::~Window()
	{
		glfwDestroyWindow(window); //selfexplanatory
		glfwTerminate();
	}

	void Window::createWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
	{
		if (glfwCreateWindowSurface(instance, window, nullptr, surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create window surface");
		}
	}

	void Window::frameBufferResizedCallback(GLFWwindow* window, int width, int height)
	{
		auto lvewindow = reinterpret_cast<Window *>(glfwGetWindowUserPointer(window));
		lvewindow->frameBufferResized = true;
		lvewindow->_width = width;
		lvewindow->_height = height;
	}
	
} 