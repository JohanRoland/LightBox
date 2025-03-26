#pragma once

#include "lightBox_camera.hpp"
#include "lightBox_pipeline.hpp"
#include "lightBox_device.hpp"
#include "lightBox_swap_chain.hpp"
#include "lightBox_game_object.hpp"
#include "ligthBox_frame_info.hpp"


#include <memory>
#include <vector>

namespace lightBox {
	class PointLightSystem {
	public:
		PointLightSystem(LightBoxDevice &device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout);
		~PointLightSystem();

		PointLightSystem(const PointLightSystem&) = delete;
		PointLightSystem operator=(const PointLightSystem&) = delete;
		void render(FrameInfo &frameInfo);

	private:
		void createPipelineLayout(VkDescriptorSetLayout globalSetLayout);
		void createPipeline(VkRenderPass renderPass);


		LightBoxDevice & lightBoxDevice;

		std::unique_ptr<LightBoxPipeline> lightBoxPipeline;
		VkPipelineLayout pipelineLayout;
	};
}