#include "first_app.hpp"

#include "lightBox_camera.hpp"
#include "lightBox_buffer.hpp"
#include "simple_render_system.hpp"
#include "keyboard_movement_controller.hpp"



#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <chrono>


namespace lightBox {

	struct GlobalUbo {
		glm::mat4 projectionViewMatrix{ 1.0f };
		glm::vec3 lightDirection = glm::normalize(glm::vec3{ 1.0f, -3.0f, -1.0f });
	};

	FirstApp::FirstApp()
	{
		globalPool = LightBoxDescriptorPool::Builder(lightBoxDevice)
			.setMaxSets(LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT)
			.build();
		loadGameObjects();
	}
	FirstApp::~FirstApp() {}
	void lightBox::FirstApp::run()
	{
		std::vector<std::unique_ptr<LightBoxBuffer>> uboBuffers(LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT);
		for (int i = 0; i < uboBuffers.size(); i++) {
			uboBuffers[i] = std::make_unique< LightBoxBuffer>(lightBoxDevice,
				sizeof(GlobalUbo),
				1,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT); // | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
			uboBuffers[i]->map();
		}
		auto gSL =  LightBoxDescriptorSetLayout::Builder(lightBoxDevice);
		auto gSLA = gSL.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT);
		auto globalSetLayout = gSLA.build();

		std::vector<VkDescriptorSet> globalDescriptorSets(LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT);

		for (int i = 0; i < globalDescriptorSets.size(); i++) {
			auto bufferInfo = uboBuffers[i]->descriptorInfo();
			LightBoxDescriptorWriter(*globalSetLayout, *globalPool)
				.writeBuffer(0, &bufferInfo)
				.build(globalDescriptorSets[i]);
		}

		SimpleRenderSystem simpleRenderSystem{ lightBoxDevice, lightBoxRenderer.getSwapChainRenderPass(), globalSetLayout->getDescriptorSetLayout()};
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
						int frameIndex = lightBoxRenderer.getFrameIndex();
						FrameInfo frameInfo{
							frameIndex,
							frameTime,
							commandBuffer,
							camera,
							globalDescriptorSets[frameIndex]
						};

						// setup buffers
						GlobalUbo ubo{};
						ubo.projectionViewMatrix = camera.getProjection() * camera.getView();
						uboBuffers[frameIndex]->writeToBuffer(&ubo);
						uboBuffers[frameIndex]->flush();


						// render 
						lightBoxRenderer.beginSwapChainRenderPass(commandBuffer);
						simpleRenderSystem.renderGameObjects(frameInfo, gameObjects);
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
		gameObject.transform.translation = { 0.25f, 0.0f, 2.5f };
		gameObject.transform.scale = { 0.5f, 0.5f, 0.5f };
		gameObjects.push_back(std::move(gameObject));

		std::shared_ptr<LightBoxModel> lightBoxModel2 = LightBoxModel::createModelFromFile(lightBoxDevice, "models/flat_vase.obj");

		auto gameObject2 = LightBoxGameObject::createGameObject();
		gameObject2.model = lightBoxModel2;
		gameObject2.transform.translation = { 0.5f, 0.0f, 2.5f };
		gameObject2.transform.scale = { 0.5f, 0.25f, 0.5f };
		gameObjects.push_back(std::move(gameObject2));
	}
}