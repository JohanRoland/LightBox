#pragma once


#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm.hpp>

namespace lightBox 
{

	class LightBoxCamera {
	public:
		void setOrthographicProjection(float left, float right, float top, float bottom, float near, float far);
		void setPerspectiveProjection(float fovy, float aspect, float near, float far);
		void updatePerspectivAspect(float aspect);

		void setViewDirection(glm::vec3 position, glm::vec3 direction, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
		void setViewTarget(glm::vec3 position, glm::vec3 target, glm::vec3 up = glm::vec3(0.0f, -1.0f, 0.0f));
		void setViewYXZ(glm::vec3 position, glm::vec3 rotation);

		const glm::mat4& getProjection() const { return projectionMatrix; }
		const glm::mat4& getView() const { return viewMatrix; }

	private:
		float fovy = 1.0f;
		float aspectRatio = 1.0f;
		float nearPlane = 0.1f;
		float farPlane = 1.0f;

		glm::mat4 projectionMatrix{ 1.0f };
		glm::mat4 viewMatrix{ 1.0f };
	};
}