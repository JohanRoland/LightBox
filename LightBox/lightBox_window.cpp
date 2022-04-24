#include "lightBox_window.h"

#include <iostream>
#include <stdexcept>
#include <vector>


//#include <glm/glm.hpp>
//#include <SDL2/SDL.h>
//#include <SDL2/SDL_syswm.h>
//#include <SDL2/SDL_vulkan.h>
#include <vulkan/vulkan.hpp>
namespace lightBox {

	LightBoxWindow::LightBoxWindow(int w, int h, std::string name) : width(w), height(h), windowName(name)
	{
		initWindow();
	}

	LightBoxWindow::~LightBoxWindow()
	{
		SDL_DestroyWindow(lightBoxWindow);
		SDL_Quit();
	}

	VkExtent2D LightBoxWindow::getExtent()
	{
		return { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}

	void LightBoxWindow::createWindowSurface(VkInstance instance, VkSurfaceKHR * surface)
	{
		bool success = SDL_Vulkan_CreateSurface(lightBoxWindow, instance, surface);
		if (success != true)
		{
			throw std::runtime_error("Failed to create Vuclan Surface!");
		}

	}

	int LightBoxWindow::initWindow()
	{
		// Create an SDL window that supports Vulkan rendering.
		if (SDL_Init(SDL_INIT_VIDEO) != 0) {
			std::cout << "Could not initialize SDL." << std::endl;
			return 1;
		}
		lightBoxWindow = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED, width, height, SDL_WINDOW_VULKAN);
		if (lightBoxWindow == NULL) {
			std::cout << "Could not create SDL window." << std::endl;
			return 1;
		}

		// Get WSI extensions from SDL (we can add more if we like - we just can't remove these)
		unsigned extension_count;
		if (!SDL_Vulkan_GetInstanceExtensions(lightBoxWindow, &extension_count, NULL)) {
			std::cout << "Could not get the number of required instance extensions from SDL." << std::endl;
			return 1;
		}
		std::vector<const char*> extensions(extension_count);
		if (!SDL_Vulkan_GetInstanceExtensions(lightBoxWindow, &extension_count, extensions.data())) {
			std::cout << "Could not get the names of required instance extensions from SDL." << std::endl;
			return 1;
		}


		return 0;
	}

	std::vector<const char *> LightBoxWindow::getRequiredExtensions(bool enableValidationLayers)
	{
		unsigned int extensionCount = 0;
		SDL_bool success = SDL_Vulkan_GetInstanceExtensions(lightBoxWindow, &extensionCount, nullptr);
		if (success != SDL_TRUE)
		{
			throw std::runtime_error("Failed to get instance number of extensions!");
		}
		std::vector<const char *> extensionNames(extensionCount);
		success = SDL_Vulkan_GetInstanceExtensions(lightBoxWindow, &extensionCount, extensionNames.data());
		if (success != SDL_TRUE)
		{
			throw std::runtime_error("Failed to get instance extensions!");
		}

		if (enableValidationLayers) {
			extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensionNames;
	}
}