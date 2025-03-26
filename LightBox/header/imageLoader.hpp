#pragma once
#include <lightBox_texture.hpp>
#include <lightBox_buffer.hpp>
#include <memory>

namespace imageLoader {

	class ImageLoader abstract {
	public:
		virtual lightBox::vKImage::Texture loadImageFromDisk(
			std::filesystem::path filePath,
			const VkPhysicalDeviceMemoryProperties& memoryProperties,
			lightBox::LightBoxDevice& device,
			VkQueue queue) = 0;
		virtual lightBox::vKImage::Texture loadImageFromMemmory(
			std::unique_ptr<std::vector<char>> buffer,
			const VkPhysicalDeviceMemoryProperties& memoryProperties,
			lightBox::LightBoxDevice& device,
			VkQueue queue) = 0;
	};
}