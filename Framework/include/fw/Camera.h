#pragma once

#include "Transformation.h"

#include <glm/glm.hpp>

namespace fw
{
class Camera
{
public:
    Camera();

    void setPosition(const glm::vec3& pos);
    void setPosition(float x, float y, float z);
    void setRotation(const glm::vec3& rot);
    void move(const glm::vec3& translation);
    void move(float x, float y, float z);
    void rotate(const glm::vec3& axis, float amount);

    const Transformation& getTransformation() const;

    const glm::mat4x4& getViewMatrix() const;
    const glm::mat4x4& getProjectionMatrix() const;

private:
    float m_FOV = 45.0f;
    float m_ratio = 1.33333f;
    float m_nearClipDistance = 0.1f;
    float m_farClipDistance = 100.0f;

    glm::mat4 m_viewMatrix;
    glm::mat4 m_projectionMatrix;

    Transformation m_transformation;

    void updateViewMatrix();
    void updateProjectionMatrix();
};

} // namespace fw
