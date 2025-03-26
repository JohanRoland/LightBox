#pragma once
#include "imageLoader.hpp"



namespace imageLoader {
	struct Buffer
	{
		VkBuffer buffer;
		VkDeviceMemory memory;
		void* data;
		size_t size;
	};
	void createTextureImage(std::filesystem::path filePath,
		const VkPhysicalDeviceMemoryProperties& memoryProperties,
		lightBox::LightBoxDevice& device,
		VkQueue queue,
		lightBox::vKImage::Texture& outTexture,
		VkBuffer stagingBuffer,
		VkDeviceMemory stagingBufferMemory);
	class DdsLoaderImplementation /*: public imageLoader::ImageLoader */ {
	public:
		void createBuffer(Buffer& result, VkDevice device, const VkPhysicalDeviceMemoryProperties& memoryProperties, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags);
//		lightBox::vKImage::Texture loadImageFromDisk(
//			std::filesystem::path filePath,
//			const VkPhysicalDeviceMemoryProperties& memoryProperties,
//			lightBox::LightBoxDevice& device,
//			VkQueue queue) override;
//		lightBox::vKImage::Texture loadImageFromMemmory(
//			std::unique_ptr<std::vector<char>> buffer,
//			const VkPhysicalDeviceMemoryProperties& memoryProperties,
//			lightBox::LightBoxDevice& device,
//			VkQueue queue) override;
	};
}