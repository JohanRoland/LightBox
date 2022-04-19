#pragma once

#include "lightBox_window.h"
#include "lightBox_pipeline.h"

namespace lightBox {
	class FirstApp {
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		void run();

	private:
		Window window{ WIDTH, HEIGHT, "Hello, first app" };
		Pipeline pipeline{ "Shaders/simple_shader.vert.spv", "Shaders/simple_shader.frag.spv" };
	};
}