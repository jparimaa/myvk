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

void Camera::move(const glm::vec3& translation)
{
    transformation.move(translation);
    updateViewMatrix();
}

void Camera::rotate(const glm::vec3& axis, float amount)
{
    transformation.rotate(axis, amount);
    updateViewMatrix();
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
