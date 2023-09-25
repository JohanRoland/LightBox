#include "lightBox_renderer.hpp"

#include <stdexcept>

namespace lightBox {

	LightBoxRenderer::LightBoxRenderer(LightBoxWindow& window, LightBoxDevice& device) 
		: lightBoxWindow(window), lightBoxDevice(device), currentFrameIndex(0)
	{
		recreateSwapChain();
		createCommandBuffers();
	}
	LightBoxRenderer::~LightBoxRenderer() 
	{
		freeComandBuffers();
	}

	VkCommandBuffer LightBoxRenderer::beginFrame()
	{
		assert(!isFrameStarted && "Can not call begin frame while frame is already in progress");
		uint32_t imageIndex;
		const VkResult resultSuccess = lightBoxSwapChain->acquireNextImage(&currentImageIndex);

		if (resultSuccess == VK_ERROR_OUT_OF_DATE_KHR)
		{
			recreateSwapChain();
			return nullptr;
		}

		if (VK_SUCCESS != resultSuccess && VK_SUBOPTIMAL_KHR != resultSuccess)
		{
			throw std::runtime_error("Failed to acquire swap chain image!");
		}

		isFrameStarted = true;

		auto commandBuffer = getCurrentCommandBuffer();

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		const VkResult beginSuccess = vkBeginCommandBuffer(commandBuffer, &beginInfo);

		if (VK_SUCCESS != beginSuccess)
		{
			throw std::runtime_error("Failed to begin recording command buffer!");
		}

		return commandBuffer;
	}

	void LightBoxRenderer::endFrame()
	{
		assert(isFrameStarted && "Can not call end frame while frame is not in progress");
		auto commandBuffer = getCurrentCommandBuffer();

		VkResult endCmdBuffsuccess = vkEndCommandBuffer(commandBuffer);
		if (VK_SUCCESS != endCmdBuffsuccess)
		{
			throw std::runtime_error("Failed to end command buffer!");
		}

		auto resultSubmitBuffer = lightBoxSwapChain->submitCommandBuffers(&commandBuffer, &currentImageIndex);
		if (VK_ERROR_OUT_OF_DATE_KHR == resultSubmitBuffer ||
			VK_SUBOPTIMAL_KHR == resultSubmitBuffer ||
			lightBoxWindow.wasWindowResized())
		{
			lightBoxWindow.resetWindowResizedFlag();
			recreateSwapChain();
		}else if (VK_SUCCESS != resultSubmitBuffer)
		{
			throw std::runtime_error("Failed to present/submit swap chain image!");
		}

		isFrameStarted = false;
		currentFrameIndex = (currentFrameIndex + 1) % LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT;
	}

	void LightBoxRenderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "cant call begin swapchain render pass while renderpass is in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cant begin render pass on a command buffer from a different frame");

		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = lightBoxSwapChain->getRenderPass();
		renderPassInfo.framebuffer = lightBoxSwapChain->getFrameBuffer(currentImageIndex);

		renderPassInfo.renderArea.offset = { 0 ,0 };
		renderPassInfo.renderArea.extent = lightBoxSwapChain->getSwapChainExtent();

		std::array<VkClearValue, 2> clearValues{};
		clearValues[0].color = { 0.01f, 0.01f, 0.01f, 1.0f };
		clearValues[1].depthStencil = { 1.0f, 0 };
		renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
		renderPassInfo.pClearValues = clearValues.data();

		vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		VkViewport viewport{};
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = static_cast<float>(lightBoxSwapChain->getSwapChainExtent().width);
		viewport.height = static_cast<float>(lightBoxSwapChain->getSwapChainExtent().height);
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

		VkRect2D sicssor{ {0,0}, lightBoxSwapChain->getSwapChainExtent() };
		vkCmdSetScissor(commandBuffer, 0, 1, &sicssor);
	}

	void LightBoxRenderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
	{
		assert(isFrameStarted && "Cant call end swapchain render pass while renderpass is in progress");
		assert(commandBuffer == getCurrentCommandBuffer() && "Cant end render pass on a command buffer from a different frame");

		vkCmdEndRenderPass(commandBuffer);

	}

	void LightBoxRenderer::createCommandBuffers()
	{
		commandBuffers.resize(LightBoxSwapChain::MAX_FRAMES_IN_FLIGHT);

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

	void LightBoxRenderer::freeComandBuffers()
	{
		vkFreeCommandBuffers(lightBoxDevice.device(), lightBoxDevice.getCommandPool(), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data());

		commandBuffers.clear();
	}

	void LightBoxRenderer::recreateSwapChain()
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
			std::shared_ptr<LightBoxSwapChain> oldSwapChain = std::move(lightBoxSwapChain);
			lightBoxSwapChain = std::make_unique<LightBoxSwapChain>(lightBoxDevice, extent, oldSwapChain);

			if (!oldSwapChain->compareSwapFormats(*lightBoxSwapChain.get())) {
				std::runtime_error("swap chain image/deapth format has changed"); //handle more gracefully later
			}
		}

		//ToDo fix
	}

}