#include "first_app.h"

#include <stdexcept>

namespace lightBox {
	FirstApp::FirstApp()
	{
		loadModels();
		createPipelineLayout();
		createPipeline();
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

				switch (event.type) {

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

	void FirstApp::loadModels()
	{
		std::vector<LightBoxModel::Vertex> vertices
		{
			{{0.0f, -0.5f}},
			{{0.5f, 0.5f}},
			{{-0.5f, 0.5f}}
		};

		lightBoxModel = std::make_unique<LightBoxModel>(lightBoxDevice, vertices);

	}

	void FirstApp::createPipelineLayout()
	{
		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = 0;
		pipelineLayoutInfo.pSetLayouts = nullptr;
		pipelineLayoutInfo.pushConstantRangeCount = 0;
		pipelineLayoutInfo.pPushConstantRanges = nullptr;
		pipelineLayoutInfo.pNext = nullptr;

		auto success = vkCreatePipelineLayout(lightBoxDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

		if (VK_SUCCESS != success)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

	}

	void FirstApp::createPipeline()
	{
		auto pipelineConfig = 
			LightBoxPipeline::defaultPipelineConfigInfo(lightBoxSwapChain.width(), lightBoxSwapChain.height());

		pipelineConfig.renderPass = lightBoxSwapChain.getRenderPass();
		pipelineConfig.pipelineLayout = pipelineLayout;
		lightBoxPipeline = std::make_unique<LightBoxPipeline>(
			lightBoxDevice,
			"Shaders/simple_shader.vert.spv",
			"Shaders/simple_shader.frag.spv",
			pipelineConfig);
	}

	void FirstApp::createCommandBuffers()
	{
		commandBuffers.resize(lightBoxSwapChain.imageCount());

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

		for (int i = 0; i < commandBuffers.size(); i++) 
		{
			VkCommandBufferBeginInfo beginInfo{};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			
			const VkResult beginSuccess = vkBeginCommandBuffer(commandBuffers[i], &beginInfo);

			if (VK_SUCCESS != beginSuccess)
			{
				throw std::runtime_error("Failed to begin recording command buffer!");
			}

			VkRenderPassBeginInfo renderPassInfo{};
			renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass = lightBoxSwapChain.getRenderPass();
			renderPassInfo.framebuffer = lightBoxSwapChain.getFrameBuffer(i);

			renderPassInfo.renderArea.offset = {0 ,0 };
			renderPassInfo.renderArea.extent = lightBoxSwapChain.getSwapChainExtent();

			std::array<VkClearValue, 2> clearValues{};
			clearValues[0].color = { 0.1f, 0.1f, 0.1f, 0.1f };
			clearValues[1].depthStencil = { 1.0f, 0};
			renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
			renderPassInfo.pClearValues = clearValues.data();

			vkCmdBeginRenderPass(commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

			lightBoxPipeline->bind(commandBuffers[i]);
			lightBoxModel->bind(commandBuffers[i]);
			lightBoxModel->draw(commandBuffers[i]);

			vkCmdEndRenderPass(commandBuffers[i]);
			VkResult endCmdBuffsuccess = vkEndCommandBuffer(commandBuffers[i]);
			if (VK_SUCCESS != endCmdBuffsuccess)
			{
				throw std::runtime_error("Failed to end command buffer!");
			}
		}


	}

	void FirstApp::drawFrame()
	{
		uint32_t imageIndex;
		const VkResult resultSuccess = lightBoxSwapChain.acquireNextImage(&imageIndex);

		if (VK_SUCCESS != resultSuccess && VK_SUBOPTIMAL_KHR != resultSuccess)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		const VkResult resultSubmitBuffer = lightBoxSwapChain.submitCommandBuffers(&commandBuffers[imageIndex], &imageIndex);
		if (VK_SUCCESS != resultSubmitBuffer)
		{
			throw std::runtime_error("Failed to present/submit swap chain image!");
		}

	}

}