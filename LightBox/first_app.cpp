#include "first_app.hpp"

#include "simple_render_system.hpp"

#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lightBox {

	FirstApp::FirstApp()
	{
		loadGameObjects();
	}
	FirstApp::~FirstApp() {}
	void lightBox::FirstApp::run()
	{
		SimpleRenderSystem simpleRenderSystem{ lightBoxDevice, lightBoxRenderer.getSwapChainRenderPass() };

		bool stillRunning = true;
		while (stillRunning) {

			SDL_Event event;
			while (SDL_PollEvent(&event)) {

				switch (event.type) 
				{
				case SDL_WINDOWEVENT:
					switch (event.window.event) 
					{
						case SDL_WINDOWEVENT_RESIZED:
							/*
							SDL_Window* win = SDL_GetWindowFromID(event->window.windowID);
							if (win == (SDL_Window*)data) {
								printf("resizing.....\n");
								int width = 0;
								int	hight = 0;
								SDL_GetWindowSize(win, &width, &hight);
								auto lightBoxWidow = reinterpret_cast<LightBoxWindow*>(SDL_GetWindowData(win, "classWindow"));
								lightBoxWidow->frameBufferResized = true;
								lightBoxWidow->width = width;
								lightBoxWidow->height = hight;
							}
							*/
						break;
						default:
							// Do nothing.
							break;
					}
					break;
				case SDL_QUIT:
					stillRunning = false;
					break;
				default:
					// Do nothing.
					break;
				}
			}
			auto commandBuffer = lightBoxRenderer.beginFrame();
			if (commandBuffer != nullptr) {
				lightBoxRenderer.beginSwapChainRenderPass(commandBuffer);
				simpleRenderSystem.renderGameObjects(commandBuffer, gameObjects);
				lightBoxRenderer.endSwapChainRenderPass(commandBuffer);
				lightBoxRenderer.endFrame();
			}
		}

		vkDeviceWaitIdle(lightBoxDevice.device());
	}

	void FirstApp::loadGameObjects()
	{
		std::vector<LightBoxModel::Vertex> vertices
		{
			{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
			{{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
			{{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
		};

		auto lightBoxModel = std::make_shared<LightBoxModel>(lightBoxDevice, vertices);

		auto triangle = LightBoxGameObject::createGameObject();
		triangle.model = lightBoxModel;
		triangle.color = { 0.1f, 0.8f, 0.1f };
		triangle.transform2D.translation.x = 0.2f;
		triangle.transform2D.scale = { 2.0f, 0.5f };
		triangle.transform2D.rotation = 0.25f * glm::two_pi<float>();

		gameObjects.push_back(std::move(triangle));
	}
}