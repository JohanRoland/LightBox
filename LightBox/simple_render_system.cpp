#include "simple_render_system.hpp"

#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lightBox {
	struct SimplePushConstantData {
	public:
		glm::mat4 transform{ 1.f };
		glm::mat4 normalMatix{ 1.0f };
	};

	SimpleRenderSystem::SimpleRenderSystem(LightBoxDevice& device, VkRenderPass renderPass) : lightBoxDevice{ device }
	{
		createPipelineLayout();
		createPipeline(renderPass);

	}
	SimpleRenderSystem::~SimpleRenderSystem()
	{
		vkDestroyPipelineLayout(lightBoxDevice.device(), pipelineLayout, nullptr);

	}


	void SimpleRenderSystem::createPipelineLayout()
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

	void SimpleRenderSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(nullptr != pipelineLayout && "PipelineLayout is null when trying to createing pipeline!\n");

		PipelineConfigurationInfo pipelineConfig{};
		LightBoxPipeline::defaultPipelineConfigInfo(pipelineConfig);

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		lightBoxPipeline = std::make_unique<LightBoxPipeline>(
			lightBoxDevice,
			"Shaders/simple_shader.vert.spv",
			"Shaders/simple_shader.frag.spv",
			pipelineConfig);
	}


	void SimpleRenderSystem::renderGameObjects(
		FrameInfo &frameInfo,
		std::vector<LightBoxGameObject>& gameObjects)
	{
		lightBoxPipeline->bind(frameInfo.commandBuffer);


		auto projectionView = frameInfo.camera.getProjection() * frameInfo.camera.getView();

		for (auto& object : gameObjects) {
			auto modelMatix = object.transform.mat4();
			SimplePushConstantData push{
				.transform{projectionView * modelMatix},
				.normalMatix{object.transform.normalMatrix()}};

			vkCmdPushConstants(
				frameInfo.commandBuffer,
				pipelineLayout,
				VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
				0,
				sizeof(SimplePushConstantData),
				&push);
			object.model->bind(frameInfo.commandBuffer);
			object.model->draw(frameInfo.commandBuffer);
		}
	}

}