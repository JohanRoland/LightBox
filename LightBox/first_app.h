#pragma once

#include "lightBox_window.h"
#include "lightBox_pipeline.h"
#include "lightBox_device.hpp"
#include "lightBox_swap_chain.hpp"
#include "lightBox_game_object.hpp"

#include <memory>
#include <vector>

namespace lightBox {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 800;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp &) = delete;
		FirstApp operator=(const FirstApp &) = delete;

		void run();

	private:
		void loadGameObjects();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void freeComandBuffers();
		void drawFrame();
		void recreateSwapChain();
		void recordCommandBuffer(int imageIndex);
		void renderGameObjects(VkCommandBuffer commandBuffer);

		LightBoxWindow lightBoxWindow{ WIDTH, HEIGHT, "Hello, first app" };
		LightBoxDevice lightBoxDevice{ lightBoxWindow,  };
		std::unique_ptr<LightBoxSwapChain> lightBoxSwapChain;
		std::unique_ptr<LightBoxPipeline> lightBoxPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::vector<LightBoxGameObject> gameObjects;

	};
}