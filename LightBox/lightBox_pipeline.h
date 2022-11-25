#pragma once

#include "lightBox_device.hpp"

#include <string>
#include <vector>

namespace lightBox {

	struct PipelineConfigurationInfo {
		PipelineConfigurationInfo(const PipelineConfigurationInfo&) = delete;
		PipelineConfigurationInfo& operator=(const PipelineConfigurationInfo&) = delete;

		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		std::vector<VkDynamicState> dynamicStateEnables;
		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo;

		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class LightBoxPipeline {
	public:
		LightBoxPipeline(
			LightBoxDevice &device,
			const std::string vertFilePath,
			const std::string fragFilePath,
			const PipelineConfigurationInfo &configInfo);
		~LightBoxPipeline(); //TODO: implement

		LightBoxPipeline(const LightBoxPipeline&) = delete;
		LightBoxPipeline &operator=(const LightBoxPipeline&) = delete;

		void bind(VkCommandBuffer commandBuffer);

		static void defaultPipelineConfigInfo(PipelineConfigurationInfo& configInfo);
	private:
		static std::vector<char> readFile(const std::string& filepath);

		void createGraphicsPipeline(
			const std::string& vetFilepath,
			const std::string& fragFilePath,
			const PipelineConfigurationInfo &configInfo);

		void createShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

		LightBoxDevice& lightBoxDevice; // Unsafe memory, relies on the dependancy between device and pipeline. TODO: Redo as shared pointer 
		VkPipeline graphicsPipeline;
		VkShaderModule vertShaderModule;
		VkShaderModule fragShaderModule;

	};
}