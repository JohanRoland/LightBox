#pragma once

#include "lightBox_window.h"
#include "lightBox_pipeline.h"
#include "lightBox_device.hpp"
#include "lightBox_swap_chain.hpp"
#include "lightBox_model.h"

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
		void loadModels();
		void createPipelineLayout();
		void createPipeline();
		void createCommandBuffers();
		void drawFrame();


		LightBoxWindow lightBoxWindow{ WIDTH, HEIGHT, "Hello, first app" };
		LightBoxDevice lightBoxDevice{ lightBoxWindow,  };
		LightBoxSwapChain lightBoxSwapChain{ lightBoxDevice, lightBoxWindow.getExtent() };
		std::unique_ptr<LightBoxPipeline> lightBoxPipeline;
		VkPipelineLayout pipelineLayout;
		std::vector<VkCommandBuffer> commandBuffers;
		std::unique_ptr<LightBoxModel> lightBoxModel;

	};
}