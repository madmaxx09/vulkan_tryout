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
	App::App()
	{
		// std::cout << "Size of app : " << sizeof(App) << std::endl;
		// std::cout << "Size of window : " << sizeof(Window) << std::endl;
		// std::cout << "Size of cam : " << sizeof(LveCamera) << std::endl;
		// std::cout << "Size of engine : " << sizeof(EngineDevice) << std::endl;
		// std::cout << "Size of renderer : " << sizeof(LveRenderer) << std::endl;
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
				lveRenderer.beginSwapchainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
				lveRenderer.endSwapchainRenderPass(commandBuffer);
				lveRenderer.endFrame();
			}
		}
		vkDeviceWaitIdle(device.device());
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