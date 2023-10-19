#pragma once

#include "lightBox_camera.hpp"
#include "lightBox_pipeline.hpp"
#include "lightBox_device.hpp"
#include "lightBox_swap_chain.hpp"
#include "lightBox_game_object.hpp"


#include <memory>
#include <vector>

namespace lightBox {
	class SimpleRenderSystem {
	public:
		SimpleRenderSystem(LightBoxDevice &device, VkRenderPass renderPass);
		~SimpleRenderSystem();

		SimpleRenderSystem(const SimpleRenderSystem&) = delete;
		SimpleRenderSystem operator=(const SimpleRenderSystem&) = delete;
		void renderGameObjects(VkCommandBuffer commandBuffer, std::vector<LightBoxGameObject> &gameObjects, const LightBoxCamera& camera);

	private:
		void createPipelineLayout();
		void createPipeline(VkRenderPass renderPass);


		LightBoxDevice & lightBoxDevice;

		std::unique_ptr<LightBoxPipeline> lightBoxPipeline;
		VkPipelineLayout pipelineLayout;
	};
}