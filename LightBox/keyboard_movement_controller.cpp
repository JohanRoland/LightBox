#include "keyboard_movement_controller.hpp"



namespace lightBox{

	void KeyboardMovementController::moveInPlaneXZ(SDL_Keycode keyCode, float dt, LightBoxGameObject& gameObject)
	{
		glm::vec3 rotate{ 0 };

		if (keys.lookRight == keyCode) rotate.y += 1;
		if (keys.lookLeft == keyCode) rotate.y -= 1;
		if (keys.lookUp == keyCode) rotate.x += 1;
		if (keys.lookDown == keyCode) rotate.x -= 1;


		if (glm::dot(rotate, rotate) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.rotation += lookSpeed * dt * glm::normalize(rotate);
		}

		gameObject.transform.rotation.x = glm::clamp(gameObject.transform.rotation.x, -1.5f, 1.5f);
		gameObject.transform.rotation.y = glm::mod(gameObject.transform.rotation.y, glm::two_pi<float>());

		float yaw = gameObject.transform.rotation.y;
		const glm::vec3 forwardDirection{ sin(yaw), 0.0f, cos(yaw) };
		const glm::vec3 rightDirection{ forwardDirection.z, 0.0f, -forwardDirection.x };
		const glm::vec3 upDirection{ 0.0f, -1.0f, 0.0f };

		glm::vec3 movementDirection{ 0.0f };
		if (keys.moveForward == keyCode) movementDirection += forwardDirection;
		if (keys.moveBackward == keyCode) movementDirection -= forwardDirection;
		if (keys.moveRight == keyCode) movementDirection += rightDirection;
		if (keys.moveLeft == keyCode) movementDirection -= rightDirection;
		if (keys.moveUp == keyCode) movementDirection += upDirection;
		if (keys.moveDown == keyCode) movementDirection -= upDirection;

		if (glm::dot(movementDirection, movementDirection) > std::numeric_limits<float>::epsilon()) {
			gameObject.transform.translation += lookSpeed * dt * glm::normalize(movementDirection);
		}


	}
}