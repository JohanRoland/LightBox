#include "first_app.hpp"

#include "lightBox_camera.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"



#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>


namespace lightBox {

	FirstApp::FirstApp()
	{
		loadGameObjects();
	}
	FirstApp::~FirstApp() {}
	void lightBox::FirstApp::run()
	{
		SimpleRenderSystem simpleRenderSystem{ lightBoxDevice, lightBoxRenderer.getSwapChainRenderPass() };
		LightBoxCamera camera{};

		auto viewerObject = LightBoxGameObject::createGameObject();
		KeyboardMovementController cameraController{};

		auto currentTime = std::chrono::high_resolution_clock::now();

		bool stillRunning = true;
		unsigned int priviusTick = SDL_GetTicks();
		while (stillRunning) {
			SDL_Event event;
			while (SDL_PollEvent(&event)) {
				unsigned int currentTick = SDL_GetTicks();
				auto delta = currentTick - priviusTick;

				auto newTime = std::chrono::high_resolution_clock::now();
				float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
				currentTime = newTime;

				switch (event.type) 
				{
				case SDL_KEYDOWN:
					cameraController.moveInPlaneXZ(event.key.keysym.sym, frameTime, viewerObject);
					camera.setViewYXZ(viewerObject.transform.translation, viewerObject.transform.rotation);
					
					break;
				case SDL_WINDOWEVENT:
					break;
				case SDL_QUIT:
					stillRunning = false;
					break;
				default:
					// Do nothing.
					break;
				}

				if (delta > 1000 / 60.0)
				{
					float aspect = lightBoxRenderer.getAspectRatio();
					//camera.setOrthographicProjection(-aspect, aspect, -1.0f, 1.0f, -1.0f, 1.0f);
					camera.setPerspectiveProjection(glm::radians(50.0f), aspect, 0.1f, 10.0f);

					priviusTick = currentTick;
					auto commandBuffer = lightBoxRenderer.beginFrame();
					if (commandBuffer != nullptr) {
						lightBoxRenderer.beginSwapChainRenderPass(commandBuffer);
						simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
						lightBoxRenderer.endSwapChainRenderPass(commandBuffer);
						lightBoxRenderer.endFrame();
					}
				}
			}
		}

		vkDeviceWaitIdle(lightBoxDevice.device());
	}


	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<LightBoxModel> lightBoxModel = LightBoxModel::createModelFromFile(lightBoxDevice, "models/smooth_vase.obj");

		auto gameObject = LightBoxGameObject::createGameObject();
		gameObject.model = lightBoxModel;
		gameObject.transform.translation = { 0.0f, 0.0f, 2.5f };
		gameObject.transform.scale = { 0.5f, 0.5f, 0.5f };
		gameObjects.push_back(std::move(gameObject));
	}
}