#pragma once

#include "lightBox_window.h"
#include "lightBox_device.hpp"
#include "lightBox_swap_chain.hpp"

#include <cassert>
#include <memory>
#include <vector>

namespace lightBox {
	class LightBoxRenderer {
	public:

		LightBoxRenderer(LightBoxWindow &window, LightBoxDevice &device);
		~LightBoxRenderer();

		LightBoxRenderer(const LightBoxRenderer&) = delete;
		LightBoxRenderer operator=(const LightBoxRenderer&) = delete;

		VkRenderPass getSwapChainRenderPass() const { return lightBoxSwapChain->getRenderPass(); }
		float getAspectRatio() const { return lightBoxSwapChain->extentAspectRatio(); }
		bool isFrameInProgress() const { return isFrameStarted; }
		VkCommandBuffer getCurrentCommandBuffer() const {
			assert(isFrameStarted && "Cannot not get command buffer when frame is not started");
			return commandBuffers[currentFrameIndex];
		}
		
		uint32_t getFrameIndex() const {
			assert(isFrameStarted && "Cannot not get frame index when frame is not started");
			return currentFrameIndex;
		}

		VkCommandBuffer beginFrame();
		void endFrame();

		void beginSwapChainRenderPass(VkCommandBuffer commandBuffer);
		void endSwapChainRenderPass(VkCommandBuffer commandBuffer);

	private:
 		void createCommandBuffers();
		void freeComandBuffers();
		void recreateSwapChain();

		LightBoxWindow &lightBoxWindow;
		LightBoxDevice &lightBoxDevice;
		std::unique_ptr<LightBoxSwapChain> lightBoxSwapChain;
		std::vector<VkCommandBuffer> commandBuffers;

		uint32_t currentImageIndex;
		uint32_t currentFrameIndex;
		bool isFrameStarted = false;

	};
}