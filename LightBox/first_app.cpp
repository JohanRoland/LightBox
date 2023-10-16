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

	// temporary helper function, creates a 1x1x1 cube centered at offset
	std::unique_ptr<LightBoxModel> createCubeModel(LightBoxDevice& device, glm::vec3 offset) {
		std::vector<LightBoxModel::Vertex> vertices{

			// left face (white)
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
			{{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
			{{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

			// right face (yellow)
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
			{{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .8f, .1f}},

			// top face (orange, remember y axis points down)
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
			{{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
			{{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

			// bottom face (red)
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
			{{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
			{{.5f, .5f, .5f}, {.8f, .1f, .1f}},

			// nose face (blue)
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
			{{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
			{{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

			// tail face (green)
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
			{{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
			{{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

		};
		for (auto& v : vertices) {
			v.position += offset;
		}
		return std::make_unique<LightBoxModel>(device, vertices);
	}

	void FirstApp::loadGameObjects()
	{
		std::shared_ptr<LightBoxModel> lightBoxModel = createCubeModel(lightBoxDevice, { 0.0f,0.0f,0.0f });

		auto cube = LightBoxGameObject::createGameObject();
		cube.model = lightBoxModel;
		cube.transform.translation = { 0.0f, 0.0f, 2.5f };
		cube.transform.scale = { 0.5f, 0.5f, 0.5f };
		gameObjects.push_back(std::move(cube));
	}
}