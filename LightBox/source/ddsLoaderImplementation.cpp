#include "ddsLoaderImplementation.hpp"
#include <lightBox_buffer.hpp>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <array>
#include <memory>
#include <lightBox_buffer.hpp>

namespace imageLoader {
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

	const unsigned int DDSCAPS2_CUBEMAP = 0x200;
	const unsigned int DDSCAPS2_VOLUME = 0x200000;

	const unsigned int DDS_DIMENSION_TEXTURE2D = 3;
	/*
	* Prodly stolen
	*/
	enum DXGI_FORMAT
	{
		DXGI_FORMAT_BC1_UNORM = 71,
		DXGI_FORMAT_BC1_UNORM_SRGB = 72,
		DXGI_FORMAT_BC2_UNORM = 74,
		DXGI_FORMAT_BC2_UNORM_SRGB = 75,
		DXGI_FORMAT_BC3_UNORM = 77,
		DXGI_FORMAT_BC3_UNORM_SRGB = 78,
		DXGI_FORMAT_BC4_UNORM = 80,
		DXGI_FORMAT_BC4_SNORM = 81,
		DXGI_FORMAT_BC5_UNORM = 83,
		DXGI_FORMAT_BC5_SNORM = 84,
		DXGI_FORMAT_BC6H_UF16 = 95,
		DXGI_FORMAT_BC6H_SF16 = 96,
		DXGI_FORMAT_BC7_UNORM = 98,
		DXGI_FORMAT_BC7_UNORM_SRGB = 99,
	};
	static unsigned int fourCC(const char(&str)[5])
	{
		return (unsigned(str[0]) << 0) | (unsigned(str[1]) << 8) | (unsigned(str[2]) << 16) | (unsigned(str[3]) << 24);
	}
	/*
	* Prodly stolen
	*/
	static VkFormat getFormat(const DDS_HEADER& header, const DDS_HEADER_DXT10& header10)
	{
		if (header.ddspf.dwFourCC == fourCC("DXT1"))
			return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
		if (header.ddspf.dwFourCC == fourCC("DXT3"))
			return VK_FORMAT_BC2_UNORM_BLOCK;
		if (header.ddspf.dwFourCC == fourCC("DXT5"))
			return VK_FORMAT_BC3_UNORM_BLOCK;

		if (header.ddspf.dwFourCC == fourCC("DX10"))
		{
			switch (header10.dxgiFormat)
			{
			case DXGI_FORMAT_BC1_UNORM:
			case DXGI_FORMAT_BC1_UNORM_SRGB:
				return VK_FORMAT_BC1_RGBA_UNORM_BLOCK;
			case DXGI_FORMAT_BC2_UNORM:
			case DXGI_FORMAT_BC2_UNORM_SRGB:
				return VK_FORMAT_BC2_UNORM_BLOCK;
			case DXGI_FORMAT_BC3_UNORM:
			case DXGI_FORMAT_BC3_UNORM_SRGB:
				return VK_FORMAT_BC3_UNORM_BLOCK;
			case DXGI_FORMAT_BC4_UNORM:
				return VK_FORMAT_BC4_UNORM_BLOCK;
			case DXGI_FORMAT_BC4_SNORM:
				return VK_FORMAT_BC4_SNORM_BLOCK;
			case DXGI_FORMAT_BC5_UNORM:
				return VK_FORMAT_BC5_UNORM_BLOCK;
			case DXGI_FORMAT_BC5_SNORM:
				return VK_FORMAT_BC5_SNORM_BLOCK;
			case DXGI_FORMAT_BC6H_UF16:
				return VK_FORMAT_BC6H_UFLOAT_BLOCK;
			case DXGI_FORMAT_BC6H_SF16:
				return VK_FORMAT_BC6H_SFLOAT_BLOCK;
			case DXGI_FORMAT_BC7_UNORM:
			case DXGI_FORMAT_BC7_UNORM_SRGB:
				return VK_FORMAT_BC7_UNORM_BLOCK;
			}
		}

		return VK_FORMAT_UNDEFINED;
	}
	/*
	* Mostly stolen
	*/
	static size_t getImageSizeBC(unsigned int width, unsigned int height, unsigned int levels, unsigned int blockSize)
	{
		size_t result = 0;

		for (unsigned int i = 0; i < levels; ++i)
		{
			result += static_cast<size_t>(((width + 3) / 4) * ((height + 3) / 4) * blockSize);

			width = width > 1 ? width / 2 : 1;
			height = height > 1 ? height / 2 : 1;
		}

		return result;
	}
	//Stolen from niagara,ToDo: Rewrite
	static uint32_t selectMemoryType(const VkPhysicalDeviceMemoryProperties& memoryProperties, uint32_t memoryTypeBits, VkMemoryPropertyFlags flags)
	{
		for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; ++i)
			if ((memoryTypeBits & (1 << i)) != 0 && (memoryProperties.memoryTypes[i].propertyFlags & flags) == flags)
				return i;

		assert(!"No compatible memory type found");
		return ~0u;
	}

	VkImageView createImageView(VkDevice device, VkImage image, VkFormat format, uint32_t baseMipLevel, uint32_t levelCount)
	{
		VkImageAspectFlags aspectMask = (format == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

		VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
		createInfo.image = image;
		createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		createInfo.format = format;
		createInfo.subresourceRange.aspectMask = aspectMask;
		createInfo.subresourceRange.baseMipLevel = baseMipLevel;
		createInfo.subresourceRange.levelCount = levelCount;
		createInfo.subresourceRange.layerCount = 1;

		VkImageView view = 0;
		VkResult res = vkCreateImageView(device, &createInfo, 0, &view);
		if (VK_SUCCESS != res)
		{
			std::cout << "vkCreateImageView failed";
		}

		return view;
	}

	//Mostly stolen from niagara,ToDo: Rewrite more
	void createImage(
		lightBox::vKImage::Texture& outTexture,
		VkDevice device,
		const VkPhysicalDeviceMemoryProperties& memoryProperties,
		uint32_t width,
		uint32_t height,
		uint32_t mipLevels,
		VkFormat format,
		VkImageUsageFlags usage)
	{
		VkImageCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };

		createInfo.imageType = VK_IMAGE_TYPE_2D;
		createInfo.format = format;
		createInfo.extent = { width, height, 1 };
		createInfo.mipLevels = mipLevels;
		createInfo.arrayLayers = 1;
		createInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		createInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		createInfo.usage = usage;
		createInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

		VkImage image = 0;
		VkResult res = vkCreateImage(device, &createInfo, 0, &image);
		if (VK_SUCCESS != res)
		{
			std::cout << "vkCreateImage failure";
		}

		VkMemoryRequirements memoryRequirements;
		vkGetImageMemoryRequirements(device, image, &memoryRequirements);

		uint32_t memoryTypeIndex = selectMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		assert(memoryTypeIndex != ~0u);

		VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		allocateInfo.allocationSize = memoryRequirements.size;
		allocateInfo.memoryTypeIndex = memoryTypeIndex;
		
		VkMemoryAllocateFlagsInfo flagInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };
		if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
		{
			allocateInfo.pNext = &flagInfo;
			flagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
			flagInfo.deviceMask = 1;
		}

		VkDeviceMemory memory = 0;
		res = vkAllocateMemory(device, &allocateInfo, nullptr, &memory);
		if (VK_SUCCESS != res)
		{
			std::cout << "vkAllocateMemory failure";
		}
		res = vkBindImageMemory(device, image, memory, 0);
		if (VK_SUCCESS != res)
		{
			std::cout << "vkAllocateMemory failure";
		}

		outTexture.image = image;
		outTexture.imageView = createImageView(device, image, format, 0, mipLevels);
		outTexture.imageMemmory = memory;
	}
	VkImageMemoryBarrier2 imageBarrier(VkImage image, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkImageLayout oldLayout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkImageLayout newLayout, VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT, uint32_t baseMipLevel = 0, uint32_t levelCount = VK_REMAINING_MIP_LEVELS);
	VkImageMemoryBarrier2 imageBarrier(VkImage image, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkImageLayout oldLayout, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask, VkImageLayout newLayout, VkImageAspectFlags aspectMask, uint32_t baseMipLevel, uint32_t levelCount)
	{
		VkImageMemoryBarrier2 result = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2 };

		result.srcStageMask = srcStageMask;
		result.srcAccessMask = srcAccessMask;
		result.dstStageMask = dstStageMask;
		result.dstAccessMask = dstAccessMask;
		result.oldLayout = oldLayout;
		result.newLayout = newLayout;
		result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		result.image = image;
		result.subresourceRange.aspectMask = aspectMask;
		result.subresourceRange.baseMipLevel = baseMipLevel;
		result.subresourceRange.levelCount = levelCount;
		result.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

		return result;
	}
	VkImageMemoryBarrier imageBarrier1(
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkImageLayout oldLayout,
		VkAccessFlags dstAccessMask,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		uint32_t baseMipLevel = 0,
		uint32_t levelCount = VK_REMAINING_MIP_LEVELS);
	VkImageMemoryBarrier imageBarrier1(
		VkImage image,
		VkAccessFlags srcAccessMask,
		VkImageLayout oldLayout,
		VkAccessFlags dstAccessMask,
		VkImageLayout newLayout,
		VkImageAspectFlags aspectMask,
		uint32_t baseMipLevel,
		uint32_t levelCount)
	{

		VkImageSubresourceRange subRange = {
			.aspectMask = aspectMask,
			.baseMipLevel = baseMipLevel,
			.levelCount = levelCount,
			.baseArrayLayer = 0, // ??
			.layerCount = 1, // ??
		};


		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

		imageBarrier.oldLayout = oldLayout;
		imageBarrier.newLayout = newLayout;
		imageBarrier.image = image;
		imageBarrier.subresourceRange = subRange;

		imageBarrier.srcAccessMask = srcAccessMask;
		imageBarrier.dstAccessMask = dstAccessMask;
		return imageBarrier;
	}

	VkBufferMemoryBarrier2 bufferBarrier(VkBuffer buffer, VkPipelineStageFlags2 srcStageMask, VkAccessFlags2 srcAccessMask, VkPipelineStageFlags2 dstStageMask, VkAccessFlags2 dstAccessMask)
	{
		VkBufferMemoryBarrier2 result = { VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER_2 };

		result.srcStageMask = srcStageMask;
		result.srcAccessMask = srcAccessMask;
		result.dstStageMask = dstStageMask;
		result.dstAccessMask = dstAccessMask;
		result.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		result.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		result.buffer = buffer;
		result.offset = 0;
		result.size = VK_WHOLE_SIZE;

		return result;
	}

	void pipelineBarrier(VkCommandBuffer commandBuffer, VkDependencyFlags dependencyFlags, size_t bufferBarrierCount, const VkBufferMemoryBarrier2* bufferBarriers, size_t imageBarrierCount, const VkImageMemoryBarrier2* imageBarriers)
	{
		VkDependencyInfo dependencyInfo = { VK_STRUCTURE_TYPE_DEPENDENCY_INFO };
		dependencyInfo.dependencyFlags = dependencyFlags;
		dependencyInfo.bufferMemoryBarrierCount = unsigned(bufferBarrierCount);
		dependencyInfo.pBufferMemoryBarriers = bufferBarriers;
		dependencyInfo.imageMemoryBarrierCount = unsigned(imageBarrierCount);
		dependencyInfo.pImageMemoryBarriers = imageBarriers;

		vkCmdPipelineBarrier2(commandBuffer, &dependencyInfo);
	}

	void pipelineBarrier1(
		VkCommandBuffer commandBuffer,
		VkDependencyFlags dependencyFlags,
		VkPipelineStageFlags srcStageMask,
		VkPipelineStageFlags dstStageMask,
		size_t bufferBarrierCount,
		const VkBufferMemoryBarrier* bufferBarriers,
		size_t imageBarrierCount,
		const VkImageMemoryBarrier* imageBarriers) 
	{
		//barrier the image into the transfer-receive layout
		vkCmdPipelineBarrier(
			commandBuffer,
			srcStageMask,
			dstStageMask,
			dependencyFlags,
			0, nullptr,
			0, nullptr,
			1,
			imageBarriers);
	}


void transitionImageLayout(lightBox::LightBoxDevice &device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout)
{

	VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

	VkImageMemoryBarrier barrier{};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout = oldLayout;
	barrier.newLayout = newLayout;
	barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	barrier.image = image;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel = 0;
	barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	barrier.subresourceRange.baseArrayLayer = 0;
	barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;


	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
		barrier.srcAccessMask = 0;
		barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

		sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else {
		throw std::invalid_argument("unsupported layout transition!");
	}

	vkCmdPipelineBarrier(
		commandBuffer,
		sourceStage,
		destinationStage,
		0,
		0, nullptr,
		0, nullptr,
		1, &barrier
	);

	device.endSingleTimeCommands(commandBuffer);
}


void createBuffer(Buffer& result, VkDevice device, const VkPhysicalDeviceMemoryProperties& memoryProperties, size_t size, VkBufferUsageFlags usage, VkMemoryPropertyFlags memoryFlags)
{
	VkBufferCreateInfo createInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	createInfo.size = size;
	createInfo.usage = usage;

	VkBuffer buffer = 0;
	vkCreateBuffer(device, &createInfo, 0, &buffer);

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device, buffer, &memoryRequirements);

	uint32_t memoryTypeIndex = selectMemoryType(memoryProperties, memoryRequirements.memoryTypeBits, memoryFlags);
	assert(memoryTypeIndex != ~0u);

	VkMemoryAllocateInfo allocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	allocateInfo.allocationSize = memoryRequirements.size;
	allocateInfo.memoryTypeIndex = memoryTypeIndex;

	VkMemoryAllocateFlagsInfo flagInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO };

	if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
	{
		allocateInfo.pNext = &flagInfo;
		flagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
		flagInfo.deviceMask = 1;
	}

	VkDeviceMemory memory = 0;
	vkAllocateMemory(device, &allocateInfo, 0, &memory);

	vkBindBufferMemory(device, buffer, memory, 0);

	void* data = 0;
	if (memoryFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
		vkMapMemory(device, memory, 0, size, 0, &data);

	result.buffer = buffer;
	result.memory = memory;
	result.data = data;
	result.size = size;
}

void copyBufferToImage(
	lightBox::LightBoxDevice &device,
	VkBuffer &buffer,
	VkImage &image,
	const DDS_HEADER const* header,
	const unsigned int blockSize) {
	VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

	size_t bufferOffset = 0;
	unsigned int mipWidth = header->dwWidth, mipHeight = header->dwHeight;
	unsigned int mipCount = header->dwMipMapCount;
	for (unsigned int i = 0; i < mipCount; ++i)
	{
		VkBufferImageCopy region = { bufferOffset, 0, 0, { VK_IMAGE_ASPECT_COLOR_BIT, i, 0, 1, }, { 0, 0, 0 }, { mipWidth, mipHeight, 1 } };
		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region); // outTexture.buffer never set

		bufferOffset += ((mipWidth + 3) / 4) * ((mipHeight + 3) / 4) * blockSize;

		mipWidth = mipWidth > 1 ? mipWidth / 2 : 1;
		mipHeight = mipHeight > 1 ? mipHeight / 2 : 1;
	}

	device.endSingleTimeCommands(commandBuffer);
}



void createTextureImage(std::filesystem::path filePath,
	const VkPhysicalDeviceMemoryProperties& memoryProperties,
	lightBox::LightBoxDevice& device,
	VkQueue queue,
	lightBox::vKImage::Texture &outTexture,
	VkBuffer stagingBuffer,
	VkDeviceMemory stagingBufferMemory)
{
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements(device.device_, stagingBuffer, &memoryRequirements);
	int memBufsize = memoryRequirements.size;

	std::ifstream file(filePath, std::ios::binary | std::ios::ate);
	std::streamsize size = file.tellg();
	file.seekg(0, std::ios::beg);

	char* buffer_p = (char*)malloc(size*sizeof(char));

	if (file.read(buffer_p, size))
	{

	}
	else {
		throw std::format_error("VK_FORMAT_UNDEFINED");
	}

	const char const* buffer_ptr = (const char const*)buffer_p;
	const size_t headerSize = sizeof(DDS_HEADER) + sizeof(DDS_HEADER_DXT10);
	size_t readSoFar = 0;
	const unsigned int const* magicNumber = (const unsigned int const*)buffer_ptr;
	readSoFar += sizeof(*magicNumber);
	if (*magicNumber != fourCC("DDS "))
		throw std::format_error("none DDS");
	const DDS_HEADER const* header = (const DDS_HEADER const*)(buffer_ptr + readSoFar);
	readSoFar += sizeof(*header);
	const DDS_HEADER_DXT10 defaultHeader10 = { 0 };
	const DDS_HEADER_DXT10 * header10 = nullptr;
	if (header->ddspf.dwFourCC == fourCC("DX10"))
	{
		header10 = (const DDS_HEADER_DXT10*)(buffer_ptr + readSoFar);
		readSoFar += sizeof(*header10);
	}
	else {
		header10 = &defaultHeader10;
	}


	if (header->dwSize != sizeof(*header) || header->ddspf.dwSize != sizeof(header->ddspf))
		throw std::format_error("header size discrepancy");

	if (header->dwCaps2 & (DDSCAPS2_CUBEMAP | DDSCAPS2_VOLUME))
		throw std::format_error("none supported DDSCAPS2_CUBEMAP or DDSCAPS2_VOLUME");

	if (header->ddspf.dwFourCC == fourCC("DX10") && header10->resourceDimension != DDS_DIMENSION_TEXTURE2D)
		throw std::format_error("none DDS_DIMENSION_TEXTURE2D with DX10");

	VkFormat format = getFormat(*header, *header10);
	if (format == VK_FORMAT_UNDEFINED)
		throw std::format_error("VK_FORMAT_UNDEFINED");

	const void* startOfImageData = (void*)(buffer_ptr + readSoFar);


	const unsigned int blockSize =
		(format == VK_FORMAT_BC1_RGBA_UNORM_BLOCK || format == VK_FORMAT_BC4_SNORM_BLOCK || format == VK_FORMAT_BC4_UNORM_BLOCK) ? 8 : 16;

	size_t imageSize = getImageSizeBC(header->dwWidth, header->dwHeight, header->dwMipMapCount, blockSize);

	if (imageSize > memBufsize)
	{
		std::cout << "imageSize > memBufsize!!\n";
	}

	void* data;
	vkMapMemory(device.device_, stagingBufferMemory, 0, imageSize, 0, &data);
	memcpy(data, startOfImageData, imageSize);

	
	createImage(outTexture, device.device_, memoryProperties, header->dwWidth, header->dwHeight, header->dwMipMapCount, format/*VK_FORMAT_R8G8B8A8_SRGB*/, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

	transitionImageLayout(device, outTexture.image, format/*VK_FORMAT_R8G8B8A8_SRGB*/, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
	copyBufferToImage(device, stagingBuffer, outTexture.image, header, blockSize);// 1/*static_cast<uint32_t>(header->dwWidth)*/, 1 /*static_cast<uint32_t>(header->dwHeight)*/); //TODO: header->dwWidth/Hight is not good!!
	transitionImageLayout(device, outTexture.image, format/*VK_FORMAT_R8G8B8A8_SRGB*/, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	vkUnmapMemory(device.device_, stagingBufferMemory);

	free(buffer_p);
}




}