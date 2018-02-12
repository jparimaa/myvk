#include "Camera.h"

#define GLM_FORCE_RADIANS
#include <glm/gtc/matrix_transform.hpp>

namespace fw
{

Camera::Camera()
{
	updateViewMatrix();
	updateProjectionMatrix();
}

void Camera::setPosition(const glm::vec3& pos)
{
    transformation.setPosition(pos);
    updateViewMatrix();
}

void Camera::setPosition(float x, float y, float z)
{
    transformation.setPosition(x, y, z);
    updateViewMatrix();
}

void Camera::setRotation(const glm::vec3& rot)
{
    transformation.setRotation(rot);
    updateViewMatrix();
}

void Camera::move(const glm::vec3& translation)
{
    transformation.move(translation);
    updateViewMatrix();
}

void Camera::move(float x, float y, float z)
{
    transformation.move(x, y, z);
    updateViewMatrix();
}

void Camera::rotate(const glm::vec3& axis, float amount)
{
    transformation.rotate(axis, amount);
    updateViewMatrix();
}

const Transformation& Camera::getTransformation() const
{
    return transformation;
}

const glm::mat4x4& Camera::getViewMatrix() const
{
	return viewMatrix;
}

const glm::mat4x4& Camera::getProjectionMatrix() const
{
	return projectionMatrix;
}

void Camera::updateViewMatrix()
{
    glm::vec3 p = transformation.getPosition();
	viewMatrix = glm::lookAt(p, p + transformation.getForward(), transformation.getUp());
}

void Camera::updateProjectionMatrix()
{
	projectionMatrix = glm::perspective(FOV, ratio, nearClipDistance, farClipDistance);
    projectionMatrix[1][1] *= -1;
}

} // namespace fw
