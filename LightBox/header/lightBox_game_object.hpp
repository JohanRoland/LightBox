#pragma once
#include "lightBox_model.hpp"

#include <glm/gtc/matrix_transform.hpp>

#include <memory>
#include <unordered_map>

namespace lightBox {

	struct TransformComponent {
		glm::vec3 translation{};
		glm::vec3 scale{ 1.0f,1.0f,1.0f };
		glm::vec3 rotation;
		// Matrix corrsponds to Translate * Ry * Rx * Rz * Scale
		// Rotations correspond to Tait-bryan angles of Y(1), X(2), Z(3)
		// Taken wholesale from tutorial 12, TODO: Remake in as own solution 
		// https://en.wikipedia.org/wiki/Euler_angles#Rotation_matrix
		glm::mat4 mat4();
		glm::mat3 normalMatrix();
	};


	class LightBoxGameObject {
	public:
		using id_t = unsigned int;
		using GameObjectMap = std::unordered_map<id_t, LightBoxGameObject>;

		static LightBoxGameObject createGameObject() {
			static id_t currentId = 0;
			return LightBoxGameObject(currentId++);
		}
		LightBoxGameObject() = delete;
		LightBoxGameObject& operator=(const LightBoxGameObject&) = delete;
		LightBoxGameObject(LightBoxGameObject&&) = default;
		LightBoxGameObject& operator=(LightBoxGameObject&&) = default;

		const id_t getId() { return id; }

		std::shared_ptr<LightBoxModel> model{};
		glm::vec3 color{};
		TransformComponent transform{};

	private:
		const id_t id;
		LightBoxGameObject(id_t objId) : id(objId) {};
	};
}