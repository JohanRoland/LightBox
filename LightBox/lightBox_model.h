#pragma once

#include "lightBox_device.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>


#include <vector>

namespace lightBox {

	class LightBoxModel {
	public:

		struct Vertex {
			glm::vec3 position;
			glm::vec3 color;


			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();
		};

		LightBoxModel(LightBoxDevice &device, const std::vector<Vertex> &vertices);
		~LightBoxModel();

		LightBoxModel(const LightBoxModel &) = delete;
		LightBoxModel &operator=(const LightBoxModel &) = delete;

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		LightBoxDevice& lightBoxDevice;
		VkBuffer vertexBuffer;
		VkDeviceMemory vertexBufferMemory;
		uint32_t vertexCount;


	};

	}