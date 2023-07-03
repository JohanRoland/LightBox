#pragma once
#include "lightBox_model.h"

#include <memory>


namespace lightBox {

	struct Transform2DComponent {
		glm::vec2 translation{};
		glm::vec2 scale{1.0f,1.0f};
		float rotation;

		glm::mat2 mat2() {
			const float sinRotation = glm::sin(rotation);
			const float cosRotation = glm::cos(rotation);
			glm::mat2 rotMatrix{ {cosRotation, sinRotation},{-sinRotation,cosRotation} };

			glm::mat2 scaleMatrix{ {scale.x, 0.0f}, {0.0f, scale.y} };
			return rotMatrix * scaleMatrix;
		}
	};


	class LightBoxGameObject {
	public:
		using id_t = unsigned int;

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
		Transform2DComponent transform2D{};

	private:
		const id_t id;
		LightBoxGameObject(id_t objId) : id(objId) {};
	};



}