#pragma once

#include "lightBox_game_object.hpp"
#include "lightBox_window.hpp"
#include "lightBox_game_object.hpp"

namespace lightBox {
    class KeyboardMovementController {
    public:
        struct KeyMappings {
            int moveLeft = SDL_KeyCode::SDLK_a;
            int moveRight = SDL_KeyCode::SDLK_d;
            int moveForward = SDL_KeyCode::SDLK_w;
            int moveBackward = SDL_KeyCode::SDLK_s;
            int moveUp = SDL_KeyCode::SDLK_e;
            int moveDown = SDL_KeyCode::SDLK_q;
            int lookLeft = SDL_KeyCode::SDLK_LEFT;
            int lookRight = SDL_KeyCode::SDLK_RIGHT;
            int lookUp = SDL_KeyCode::SDLK_UP;
            int lookDown = SDL_KeyCode::SDLK_DOWN;
        };

        // ToDo: Make dependency injected base interface to abstract away SDL
        void moveInPlaneXZ(SDL_Keycode keyCode, float dt, LightBoxGameObject& gameObject);

        KeyMappings keys {};

        float moveSpeed{ 1.0f };
        float lookSpeed{ 0.5f };

    };
}