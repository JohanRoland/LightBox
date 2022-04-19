#pragma once


#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <SDL2/SDL_vulkan.h>

#include <string>
#include <vector>

namespace lightBox {

	class Window {
	public:
		Window(int w, int h, std::string name);
		~Window();

		Window(const Window &) = delete;
		Window operator=(const Window &) = delete;

		void createWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
		std::vector<const char*> getRequiredExtensions(bool enableValidationLayers);

	private:
		int initWindow();



		const int width;
		const int height;

		std::string windowName;
		SDL_Window* window;

	};

}