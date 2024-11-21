#include "app.hpp"
#include <array>
#include <iostream>
#include "simple_render_system.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace wind
{
	App::App()
	{
		LoadGameObjects();
	}

	App::~App()
	{
	}

	void App::run()
	{
		SimpleRenderSystem simpleRenderSystem{device, lveRenderer.getSwapChainRenderPass()};

		while(!appWindow.shouldClose())
		{
			glfwPollEvents(); //get events like keystrokes but in our case mostly clicking x button
			
			if (auto commandBuffer = lveRenderer.beginFrame())
			{
				lveRenderer.beginSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				lveRenderer.endSwapchainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(device.device());
	}

	void App::LoadGameObjects()
	{
		std::vector<LveModel::Vertex> vertices {
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto model = std::make_shared<LveModel>(device, vertices);

		auto triangle = LveGameObject::createGameObject();
		triangle.model = model;
		triangle.color = {.1f, .8f, .1f};
		triangle.transform2d.translation.x = .2f;
		triangle.transform2d.scale = {2.f, .5f};
		triangle.transform2d.rotation = .25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}
}