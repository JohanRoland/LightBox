#include "first_app.h"

#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lightBox {
	struct SimplePushConstantData {
	public:
		glm::mat2 transform{1.f};
		glm::vec2 offset{};
		alignas(16) glm::vec3 color{};
	};

	FirstApp::FirstApp()
	{
		loadGameObjects();
		createPipelineLayout();
		recreateSwapChain();
		createCommandBuffers();

	}
	FirstApp::~FirstApp()
	{
		vkDestroyPipelineLayout(lightBoxDevice.device(), pipelineLayout, nullptr);

	}
	void lightBox::FirstApp::run()
	{
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
			drawFrame();
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

	void FirstApp::createPipelineLayout()
	{

		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(SimplePushConstantData);

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 1;
		pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
		pipelineLayoutInfo.pNext = nullptr;

		auto success = vkCreatePipelineLayout(lightBoxDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

		if (VK_SUCCESS != success)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

	}

	void FirstApp::createPipeline()
	{
		assert(nullptr != lightBoxSwapChain && "Swap chain is null when trying to createing pipeline!\n");
		assert(nullptr != pipelineLayout && "PipelineLayout is null when trying to createing pipeline!\n");

		PipelineConfigurationInfo pipelineConfig{};
		LightBoxPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = lightBoxSwapChain->getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lightBoxPipeline = std::make_unique<LightBoxPipeline>(
			lightBoxDevice,
			"Shaders/simple_shader.vert.spv",
			"Shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::createCommandBuffers()
	{
		commandBuffers.resize(lightBoxSwapChain->imageCount());

		VkCommandBufferAllocateInfo allocInfo{};
		
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = lightBoxDevice.getCommandPool();
		allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

		const VkResult allocateSuccess = vkAllocateCommandBuffers(lightBoxDevice.device(), &allocInfo, commandBuffers.data());

		if (VK_SUCCESS != allocateSuccess)
		{
			throw std::runtime_error("Failed to allocate command buffers!");
		}

	}

	void FirstApp::freeComandBuffers()
	{
		vkFreeCommandBuffers(lightBoxDevice.device(), lightBoxDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		commandBuffers.clear();
	}

	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		const VkResult resultSuccess = lightBoxSwapChain->acquireNextImage(&imageIndex);

		if (resultSuccess == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return;
		}

		if (VK_SUCCESS != resultSuccess && VK_SUBOPTIMAL_KHR != resultSuccess)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		recordCommandBuffer(imageIndex);
		const VkResult resultSubmitBuffer = lightBoxSwapChain->submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (VK_ERROR_OUT_OF_DATE_KHR == resultSubmitBuffer ||
			VK_SUBOPTIMAL_KHR == resultSubmitBuffer ||
			lightBoxWindow.wasWindowResized()) 
		{
			lightBoxWindow.resetWindowResizedFlag();
			recreateSwapChain();
			return;
		}
		if (VK_SUCCESS != resultSubmitBuffer)
		{
			throw std::runtime_error("Failed to present/submit swap chain image!");
		}

	}

	void FirstApp::recreateSwapChain()
	{
		auto extent = lightBoxWindow.getExtent();
		while (0 == extent.width || 0 == extent.height) {
			extent = lightBoxWindow.getExtent();
			SDL_WaitEvent(NULL); // Possibly wrong check
		}

		vkDeviceWaitIdle(lightBoxDevice.device());
		
		if (lightBoxSwapChain == nullptr) {
			lightBoxSwapChain = std::make_unique<LightBoxSwapChain>(lightBoxDevice, extent);
		}
		else {
			lightBoxSwapChain = std::make_unique<LightBoxSwapChain>(lightBoxDevice, extent, std::move(lightBoxSwapChain));

			if (lightBoxSwapChain->imageCount() != commandBuffers.size()) {
				freeComandBuffers();
				createCommandBuffers();
			}
		}

		createPipeline();
	}

	void FirstApp::recordCommandBuffer(int imageIndex)
	{
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		const VkResult beginSuccess = vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo);

		if (VK_SUCCESS != beginSuccess)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lightBoxSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lightBoxSwapChain->getFrameBuffer(imageIndex);

		renderPassInfo.renderArea.offset = { 0 ,0 };
		renderPassInfo.renderArea.extent = lightBoxSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lightBoxSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lightBoxSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);

		VkRect2D sicssor{ {0,0}, lightBoxSwapChain->getSwapChainExtent() };
		vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &sicssor);

		
		renderGameObjects(commandBuffers[imageIndex]);



		vkCmdEndRenderPass(commandBuffers[imageIndex]);
		VkResult endCmdBuffsuccess = vkEndCommandBuffer(commandBuffers[imageIndex]);
		if (VK_SUCCESS != endCmdBuffsuccess)
		{
			throw std::runtime_error("Failed to end command buffer!");
		}
	}

	void FirstApp::renderGameObjects(VkCommandBuffer commandBuffer)
	{
		lightBoxPipeline->bind(commandBuffer);
		for (auto& object : gameObjects) {

			SimplePushConstantData push{
				.transform{object.transform2D.mat2()},
				.offset{object.transform2D.translation},
				.color{object.color}};

			vkCmdPushConstants(
				commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			object.model->bind(commandBuffer);
			object.model->draw(commandBuffer);
		}
	}

}