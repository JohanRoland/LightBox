#pragma once

#include "lightBox_window.h"
#include "lightBox_device.hpp"
#include "lightBox_swap_chain.hpp"
#include "lightBox_game_object.hpp"
#include "lightBox_renderer.hpp"

#include <memory>
#include <vector>

namespace lightBox {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 800;

		FirstApp();
		~FirstApp();

		FirstApp(const FirstApp &) = delete;
		FirstApp operator=(const FirstApp &) = delete;

		void run();

	private:
		void loadGameObjects();

		LightBoxWindow lightBoxWindow{ WIDTH, HEIGHT, "Hello, first app" };
		LightBoxDevice lightBoxDevice{ lightBoxWindow,  };
		LightBoxRenderer lightBoxRenderer{ lightBoxWindow, lightBoxDevice };

		std::vector<LightBoxGameObject> gameObjects;

	};
}