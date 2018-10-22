#pragma once

#include "Constants.h"

#include <glm/glm.hpp>

namespace fw
{
class Transformation
{
public:
    Transformation();
    void setPosition(const glm::vec3& pos);
    void setPosition(float x, float y, float z);
    void setRotation(const glm::vec3& rot);
    void setRotation(float x, float y, float z);
    void setScale(float s);
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
    glm::vec3 m_position = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 m_forward = Constants::forward;
    glm::vec3 m_up = Constants::up;
    glm::vec3 m_left = Constants::left;
    glm::mat4x4 m_worldMatrix;

    void updateWorldMatrix();
};

} // namespace fw
