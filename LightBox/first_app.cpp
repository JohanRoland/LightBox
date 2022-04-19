#include "first_app.h"

namespace lightBox {

	void lightBox::FirstApp::run()
	{
		bool stillRunning = true;
		while (stillRunning) {

			SDL_Event event;
			while (SDL_PollEvent(&event)) {

				switch (event.type) {

				case SDL_QUIT:
					stillRunning = false;
					break;
				default:
					// Do nothing.
					break;
				}
			}

			SDL_Delay(10);
		}
	}

}