#include "Transformation.h"

#define GLM_FORCE_RADIANS
#include <glm/gtx/transform.hpp>
#include <glm/gtx/euler_angles.hpp>

namespace fw
{

void Transformation::setPosition(const glm::vec3& pos)
{
    position = pos;
    updateWorldMatrix();
}

void Transformation::setPosition(float x, float y, float z)
{
    position = glm::vec3(x, y, z);
    updateWorldMatrix();
}

void Transformation::setRotation(const glm::vec3& rot)
{
    rotation = rot;
    updateWorldMatrix();
}

void Transformation::setRotation(float x, float y, float z)
{
    rotation = glm::vec3(x, y, z);
    updateWorldMatrix();
}

void Transformation::setScale(float s)
{
    scale.x = s;
    scale.y = s;
    scale.z = s;
    updateWorldMatrix();
}

void Transformation::move(const glm::vec3& translation)
{
	position += translation;
    updateWorldMatrix();
}

void Transformation::move(float x, float y, float z)
{
	position += glm::vec3(x, y, z);
    updateWorldMatrix();
}

void Transformation::rotate(const glm::vec3& axis, float amount)
{
	rotation += axis * amount;
    updateWorldMatrix();
}

void Transformation::rotateUp(float amount)
{
	rotation += Constants::up * amount;
    updateWorldMatrix();
}

glm::vec3 Transformation::getPosition() const
{
    return position;
}

glm::vec3 Transformation::getRotation() const
{
    return rotation;
}

glm::vec3 Transformation::getForward() const
{
	glm::vec4 v =
		glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
		glm::vec4(Constants::forward.x, Constants::forward.y, Constants::forward.z, 0.0f);
	return glm::vec3(v.x, v.y, v.z);
}

glm::vec3 Transformation::getUp() const
{
	glm::vec4 v =
		glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
		glm::vec4(Constants::up.x, Constants::up.y, Constants::up.z, 0.0f);
	return glm::vec3(v.x, v.y, v.z);
}

glm::vec3 Transformation::getLeft() const
{
	glm::vec4 v =
		glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
		glm::vec4(Constants::left.x, Constants::left.y, Constants::left.z, 0.0f);
	return glm::vec3(v.x, v.y, v.z);
}

const glm::mat4x4& Transformation::getWorldMatrix() const
{
	return worldMatrix;
}

void Transformation::updateWorldMatrix()
{
	worldMatrix =
		glm::translate(position) *
		glm::yawPitchRoll(rotation.y, rotation.x, rotation.z) *
		glm::scale(scale);
}

} // namespace fw
