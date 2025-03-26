#include "keyboard_movement_controller.hpp"
#include <glm/gtc/quaternion.hpp>


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
			gameObject.transform.translation += moveSpeed * dt * glm::normalize(movementDirection);
		}


	}
	void KeyboardMovementController::moveInPlaneXYZQuart(SDL_Keycode keyCode, float dt, LightBoxGameObject& gameObject)
	{
		float heading = 0;
		float attitude = 0;
		float bank = 0;

		if (keys.lookRight == keyCode) heading += 1;
		if (keys.lookLeft == keyCode) heading -= 1;
		if (keys.lookUp == keyCode) attitude += 1;
		if (keys.lookDown == keyCode) bank -= 1;

		const float c1 = cos(heading / 2);
		const float c2 = cos(attitude / 2);
		const float c3 = cos(bank / 2);

		const float s1 = sin(heading / 2);
		const float s2 = sin(attitude / 2);
		const float s3 = sin(bank / 2);

		const float w = c1 * c2 * c3 - s1 * s2 * s3;
		const float x = s1 * s2 * c3 + c1 * c2 * s3;
		const float y = s1 * c2 * c3 + c1 * s2 * s3;
		const float z = c1 * s2 * c3 - s1 * c2 * s3;

		gameObject.transform.quaternionRotation *= glm::quat(w, x, y, z);

	}
}