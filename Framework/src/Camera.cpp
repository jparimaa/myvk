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
    m_transformation.setPosition(pos);
    updateViewMatrix();
}

void Camera::setPosition(float x, float y, float z)
{
    m_transformation.setPosition(x, y, z);
    updateViewMatrix();
}

void Camera::setRotation(const glm::vec3& rot)
{
    m_transformation.setRotation(rot);
    updateViewMatrix();
}

void Camera::move(const glm::vec3& translation)
{
    m_transformation.move(translation);
    updateViewMatrix();
}

void Camera::move(float x, float y, float z)
{
    m_transformation.move(x, y, z);
    updateViewMatrix();
}

void Camera::rotate(const glm::vec3& axis, float amount)
{
    m_transformation.rotate(axis, amount);
    updateViewMatrix();
}

const Transformation& Camera::getTransformation() const
{
    return m_transformation;
}

const glm::mat4x4& Camera::getViewMatrix() const
{
    return m_viewMatrix;
}

const glm::mat4x4& Camera::getProjectionMatrix() const
{
    return m_projectionMatrix;
}

void Camera::setNearClipDistance(float distance)
{
    m_nearClipDistance = distance;
    updateProjectionMatrix();
}

void Camera::setFarClipDistance(float distance)
{
    m_farClipDistance = distance;
    updateProjectionMatrix();
}

float Camera::getNearClipDistance() const
{
    return m_nearClipDistance;
}

float Camera::getFarClipDistance() const
{
    return m_farClipDistance;
}

void Camera::updateViewMatrix()
{
    glm::vec3 p = m_transformation.getPosition();
    m_viewMatrix = glm::lookAt(p, p + m_transformation.getForward(), m_transformation.getUp());
}

void Camera::updateProjectionMatrix()
{
    m_projectionMatrix = glm::perspective(m_FOV, m_ratio, m_nearClipDistance, m_farClipDistance);
    m_projectionMatrix[1][1] *= -1;
}

} // namespace fw
