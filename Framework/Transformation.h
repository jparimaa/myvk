#pragma once

#include "Constants.h"

#include <glm/glm.hpp>

namespace fw
{

class Transformation
{
public:
    void setPosition(const glm::vec3& pos);
    void setPosition(float x, float y, float z);
    void setRotation(const glm::vec3& rot);
    void setRotation(float x, float y, float z);
	void move(const glm::vec3& translation);
    void move(float x, float y, float z);
	void rotate(const glm::vec3& axis, float amount);
    void rotateUp(float amount);

    glm::vec3 getPosition() const;
    glm::vec3 getRotation() const;
	glm::vec3 getForward() const;
	glm::vec3 getUp() const;
	glm::vec3 getLeft() const;

	const glm::mat4x4& getWorldMatrix() const;

private:
	glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 rotation = glm::vec3(0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f, 1.0f, 1.0f);
	glm::vec3 forward = Constants::forward;
	glm::vec3 up = Constants::up;
	glm::vec3 left = Constants::left;
    glm::mat4x4 worldMatrix;

    void updateWorldMatrix();
};

} // namespace fw
