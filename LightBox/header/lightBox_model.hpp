#pragma once

#include "lightBox_device.hpp"
#include "lightBox_buffer.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <memory>
#include <vector>

namespace lightBox {

	class LightBoxModel {
	public:
		struct Vertex {
			glm::vec3 position{};
			glm::vec3 color{255.0f,124.0f ,124.0f };
			glm::vec3 normal{};
			bool hasTexture = false;
			glm::vec2 uv{}; // textureCoords 

			static std::vector<VkVertexInputBindingDescription> getBindingDescriptions();
			static std::vector<VkVertexInputAttributeDescription> getAttributeDescriptions();

			bool operator==(const Vertex& other) const {
				return position == other.position
					&& color == other.color
					&& normal == other.normal
					&& uv == other.uv;
			}
		};

		struct Builder {
			std::vector<Vertex> vertices{};
			std::vector<uint32_t> indices{};

			void loadModel(const std::string& filePath);
		};


		LightBoxModel(LightBoxDevice &device, const LightBoxModel::Builder & builder);
		~LightBoxModel();

		LightBoxModel(const LightBoxModel &) = delete;
		LightBoxModel &operator=(const LightBoxModel &) = delete;

		static std::unique_ptr<LightBoxModel> createModelFromFile(LightBoxDevice& device, const std::string& filePath);

		void bind(VkCommandBuffer commandBuffer);
		void draw(VkCommandBuffer commandBuffer);
	private:
		void createVertexBuffers(const std::vector<Vertex> &vertices);
		void createIndexBuffers(const std::vector<uint32_t>& indices);

		LightBoxDevice& lightBoxDevice;
		std::unique_ptr<LightBoxBuffer> vertexBuffer;
		uint32_t vertexCount;

		bool hasIndexBuffer = false;
		std::unique_ptr<LightBoxBuffer> indexBuffer;
		uint32_t indexCount;

	};

	}