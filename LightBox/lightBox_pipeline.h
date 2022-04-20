#pragma once

#include "lightBox_device.hpp"

#include <string>
#include <vector>

namespace lightBox {

	struct PipelineConfigurationInfo {
		VkViewport viewport;
		VkRect2D scissor;
		VkPipelineViewportStateCreateInfo viewportInfo;
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		VkPipelineRasterizationStateCreateInfo rasterizationInfo;
		VkPipelineMultisampleStateCreateInfo multisampleInfo;
		VkPipelineColorBlendAttachmentState colorBlendAttachment;
		VkPipelineColorBlendStateCreateInfo colorBlendInfo;
		VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
		VkPipelineLayout pipelineLayout = nullptr;
		VkRenderPass renderPass = nullptr;
		uint32_t subpass = 0;
	};

	class Pipeline {
	public:
		Pipeline(
			LightBoxDevice &device,
			const std::string vertFilePath,
			const std::string fragFilePath,
			const PipelineConfigurationInfo &configInfo);
		~Pipeline(); //TODO: implement

		Pipeline(const Pipeline&) = delete;
		void operator=(const Pipeline&) = delete;

		static PipelineConfigurationInfo defaultPipelineConfigInfo(uint32_t width, uint32_t height);
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