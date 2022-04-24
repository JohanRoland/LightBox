#pragma once

#include <vulkan/vulkan.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include <string>
#include <vector>

namespace lightBox {

	class LightBoxWindow {
	public:
		LightBoxWindow(int w, int h, std::string name);
		~LightBoxWindow();

		LightBoxWindow(const LightBoxWindow &) = delete;
		LightBoxWindow operator=(const LightBoxWindow &) = delete;

		VkExtent2D getExtent();

		void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
		std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

	private:
		int initWindow();



		const int width;
		const int height;

		std::string windowName;
		SDL_Window* lightBoxWindow;

	};

}