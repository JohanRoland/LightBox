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
	/*****
	* The last component of the light vectors control light intensity.
	* 
	*/
	struct GlobalUbo {
		
		glm::mat4 projectionMatrix{ 1.0f };
		glm::mat4 viewMatrix{ 1.0f };
		glm::vec4 ambientLightColor{ 1.0f, 1.0f, 1.0f, 0.02f };
		glm::vec3 lightPosition{ -1.0f };
		alignas(16) glm::vec4 lightColor{ 1.0f };
	};

	FirstApp::FirstApp()
	{
		gameObjects = LightBoxGameObject::GameObjectMap();
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

		auto globalSetLayout = 
			LightBoxDescriptorSetLayout::Builder(lightBoxDevice)
			.addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			.build();

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
		viewerObject.transform.translation.z = -2.5f;
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
							globalDescriptorSets[frameIndex],
							gameObjects
						};

						// setup buffers
						GlobalUbo ubo{};
						ubo.projectionMatrix = camera.getProjection();
						ubo.viewMatrix =camera.getView();

						uboBuffers[frameIndex]->writeToBuffer(&ubo);
						uboBuffers[frameIndex]->flush();


						// render 
						lightBoxRenderer.beginSwapChainRenderPass(commandBuffer);
						simpleRenderSystem.renderGameObjects(frameInfo);
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

		auto gameObjectVaseSmooth = LightBoxGameObject::createGameObject();
		gameObjectVaseSmooth.model = lightBoxModel;
		gameObjectVaseSmooth.transform.translation = { -0.25f, 0.0f, 0.0f };
		gameObjectVaseSmooth.transform.scale = { 0.5f, 0.5f, 0.5f };
		gameObjects.emplace(gameObjectVaseSmooth.getId(), std::move(gameObjectVaseSmooth));

		lightBoxModel = LightBoxModel::createModelFromFile(lightBoxDevice, "models/flat_vase.obj");
		auto gameObjectVaseFlat = LightBoxGameObject::createGameObject();
		gameObjectVaseFlat.model = lightBoxModel;
		gameObjectVaseFlat.transform.translation = { 0.5f, 0.0f, 0.0f };
		gameObjectVaseFlat.transform.scale = { 0.5f, 0.25f, 0.5f };
		gameObjects.emplace(gameObjectVaseFlat.getId(), std::move(gameObjectVaseFlat));

		lightBoxModel = LightBoxModel::createModelFromFile(lightBoxDevice, "models/quad.obj");
		auto gameObjecPlane = LightBoxGameObject::createGameObject();
		gameObjecPlane.model = lightBoxModel;
		gameObjecPlane.transform.translation = { 0.0f, 0.0f, 0.0f };
		gameObjecPlane.transform.scale = { 4.0f, 1.0f, 3.0f };
		gameObjects.emplace(gameObjecPlane.getId(), std::move(gameObjecPlane));
	}
}