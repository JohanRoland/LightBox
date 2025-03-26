#include "renderSystems/point_light_system.hpp"

#include <stdexcept>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

namespace lightBox {
	PointLightSystem::PointLightSystem(LightBoxDevice& device, VkRenderPass renderPass, VkDescriptorSetLayout globalSetLayout) : lightBoxDevice{ device }
	{
		createPipelineLayout(globalSetLayout);
		createPipeline(renderPass);

	}
	PointLightSystem::~PointLightSystem()
	{
		vkDestroyPipelineLayout(lightBoxDevice.device(), pipelineLayout, nullptr);

	}


	void PointLightSystem::createPipelineLayout(VkDescriptorSetLayout globalSetLayout)
	{
		/*
		VkPushConstantRange pushConstantRange{};
		pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		pushConstantRange.offset = 0;
		pushConstantRange.size = sizeof(PointLightSystem);
		*/
		std::vector<VkDescriptorSetLayout> descriptorSetLayout{ globalSetLayout };

		VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
		pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayout.size());
		pipelineLayoutInfo.pSetLayouts = descriptorSetLayout.data();
		pipelineLayoutInfo.pushConstantRangeCount = 0; //  1;
		pipelineLayoutInfo.pPushConstantRanges = nullptr; // &pushConstantRange;
		pipelineLayoutInfo.pNext = nullptr;

		auto success = vkCreatePipelineLayout(lightBoxDevice.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout);

		if (VK_SUCCESS != success)
		{
			throw std::runtime_error("Failed to create pipeline layout");
		}

	}

	void PointLightSystem::createPipeline(VkRenderPass renderPass)
	{
		assert(nullptr != pipelineLayout && "PipelineLayout is null when trying to createing pipeline!\n");

		PipelineConfigurationInfo pipelineConfig{};
		LightBoxPipeline::defaultPipelineConfigInfo(pipelineConfig);

		// Not needed or used with billboards
		pipelineConfig.attributeDescriptions.clear();
		pipelineConfig.bindingDescriptions.clear();

		pipelineConfig.renderPass = renderPass;
		pipelineConfig.pipelineLayout = pipelineLayout;
		lightBoxPipeline = std::make_unique<LightBoxPipeline>(
			lightBoxDevice,
			/*
			"Shaders/point_light.vert.spv",
			"Shaders/point_light.frag.spv",
			*/
			"E:/OldInstallFiles/Users/Johan Ekdahl/source/repos/LightBox/LightBox/shaders/point_light.vert.spv",
			"E:/OldInstallFiles/Users/Johan Ekdahl/source/repos/LightBox/LightBox/shaders/point_light.frag.spv",
			pipelineConfig);
	}


	void PointLightSystem::render(FrameInfo &frameInfo)
	{
		lightBoxPipeline->bind(frameInfo.commandBuffer);
		const uint32_t uintFirstSetIndex = 0;
		const uint32_t descriptorSetCount = 1;
		const uint32_t dynamicOffsetCount = 0;
		const uint32_t *dynamicOffsetPointer = nullptr;
		vkCmdBindDescriptorSets(
			frameInfo.commandBuffer,
			VK_PIPELINE_BIND_POINT_GRAPHICS,
			pipelineLayout,
			uintFirstSetIndex,
			descriptorSetCount,
			&frameInfo.globalDescriptorSet,
			dynamicOffsetCount,
			dynamicOffsetPointer);

		vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
	}

}