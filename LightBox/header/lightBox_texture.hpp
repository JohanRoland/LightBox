#pragma once
#include <map>
#include <string>
#include <memory>
#include <lightBox_device.hpp>
#include <filesystem>
#include <lightBox_buffer.hpp>


namespace lightBox {

	namespace vKImage {
		/*
		struct textureInputChunk 
		{
			const char* filename;
			vk::Device logicalDevice;
			vk::PhysicalDevice physicalDevice;
			vk::CommandBuffer commandBuffer;
			vk::Queue queue;
			vk::DescriptorSetLayout layout;
			vk::DescriptorPool descriptorPool;

		};*/
		struct DDS_PIXELFORMAT
		{
			unsigned int dwSize;
			unsigned int dwFlags;
			unsigned int dwFourCC;
			unsigned int dwRGBBitCount;
			unsigned int dwRBitMask;
			unsigned int dwGBitMask;
			unsigned int dwBBitMask;
			unsigned int dwABitMask;
		};

		struct DDS_HEADER
		{
			unsigned int dwSize;
			unsigned int dwFlags;
			unsigned int dwHeight;
			unsigned int dwWidth;
			unsigned int dwPitchOrLinearSize;
			unsigned int dwDepth;
			unsigned int dwMipMapCount;
			unsigned int dwReserved1[11];
			DDS_PIXELFORMAT ddspf;
			unsigned int dwCaps;
			unsigned int dwCaps2;
			unsigned int dwCaps3;
			unsigned int dwCaps4;
			unsigned int dwReserved2;
		};

		struct DDS_HEADER_DXT10
		{
			unsigned int dxgiFormat;
			unsigned int resourceDimension;
			unsigned int miscFlag;
			unsigned int arraySize;
			unsigned int miscFlags2;
		};

		class Texture
		{

		public:
			unsigned int width, height, numberOfChannels, mipLevels;
			unsigned int levelCount; // Todo: what is this?
//			std::string filename;
			//std::unique_ptr<std::vector<char>> pixels;
			//std::array<char, (sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10)) / sizeof(char)> header;

//			vk::Device logicalDevice;
//			vk::PhysicalDevice physicalDevice;

			//Resorces
			VkImage image;
			VkDeviceMemory imageMemmory;
			VkImageView imageView;
//			vk::Sampler sampler;
//			LightBoxBuffer buffer;

			//Descriptors
//			vk::DescriptorSetLayout layout;
//			vk::DescriptorSet descriptorSet;
//			vk::DescriptorPool descriptorPool;


			//command objects
//			VkCommandBuffer commandBuffer;
//			vk::Queue queue;
		};
	}
}
