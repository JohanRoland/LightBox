#pragma once

#include "lightBox_window.hpp"
#include "lightBox_device.hpp"
#include "lightBox_swap_chain.hpp"
#include "lightBox_game_object.hpp"
#include "lightBox_renderer.hpp"
#include "lightBox_descriptors.hpp"
#include "lightBox_camera.hpp"
#include "keyboard_movement_controller.hpp"
#include "lightBox_texture.hpp"



#include <memory>
#include <vector>
#include <filesystem>


namespace lightBox {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 800;

		FirstApp();
		~FirstApp();
		LightBoxCamera camera;
		KeyboardMovementController cameraController = {};
		LightBoxGameObject viewerObject = LightBoxGameObject::createGameObject();




		FirstApp(const FirstApp &) = delete;
		FirstApp operator=(const FirstApp &) = delete;

		void run();

	private:

		bool loadSceen(std::filesystem::path path);
		void loadGameObjects();
		void loadTextures();
		void mapTextures();

		LightBoxWindow lightBoxWindow{ WIDTH, HEIGHT, "Hello, first app" };
		LightBoxDevice lightBoxDevice{ lightBoxWindow};
		LightBoxRenderer lightBoxRenderer{ lightBoxWindow, lightBoxDevice };

		std::unique_ptr<LightBoxDescriptorPool> globalPool{};
		LightBoxGameObject::GameObjectMap gameObjects;
		std::vector<vKImage::Texture> textures;

	};
}