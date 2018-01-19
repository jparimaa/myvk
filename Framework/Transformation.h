#pragma once

#include "Constants.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace fw
{

class Transformation
{
public:
	void move(const glm::vec3& translation);
	void rotate(const glm::vec3& axis, float amount);

	glm::vec3 getForward() const;
	glm::vec3 getUp() const;
	glm::vec3 getLeft() const;

	const glm::mat4x4& getModelMatrix() const;

private:    
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 forward = Constants::forward;
	glm::vec3 up = Constants::up;
	glm::vec3 left = Constants::left;
    glm::mat4x4 modelMatrix;

    void updateModelMatrix();
};

} // namespace fw
