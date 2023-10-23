#pragma once

#include "lightBox_camera.hpp"


#include <vulkan/vulkan.h>


namespace lightBox {
	struct FrameInfo
	{
		int frameIndex;
		float frameTime;
		VkCommandBuffer commandBuffer;
		LightBoxCamera &camera;

	};
}