#include "Transformation.h"

#define GLM_FORCE_RADIANS
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>

namespace fw
{
void Transformation::setPosition(const glm::vec3& pos)
{
    m_position = pos;
    updateWorldMatrix();
}

void Transformation::setPosition(float x, float y, float z)
{
    m_position = glm::vec3(x, y, z);
    updateWorldMatrix();
}

void Transformation::setRotation(const glm::vec3& rot)
{
    m_rotation = rot;
    updateWorldMatrix();
}

void Transformation::setRotation(float x, float y, float z)
{
    m_rotation = glm::vec3(x, y, z);
    updateWorldMatrix();
}

void Transformation::setScale(float s)
{
    m_scale.x = s;
    m_scale.y = s;
    m_scale.z = s;
    updateWorldMatrix();
}

void Transformation::move(const glm::vec3& translation)
{
    m_position += translation;
    updateWorldMatrix();
}

void Transformation::move(float x, float y, float z)
{
    m_position += glm::vec3(x, y, z);
    updateWorldMatrix();
}

void Transformation::rotate(const glm::vec3& axis, float amount)
{
    m_rotation += axis * amount;
    updateWorldMatrix();
}

void Transformation::rotateUp(float amount)
{
    m_rotation += Constants::up * amount;
    updateWorldMatrix();
}

glm::vec3 Transformation::getPosition() const
{
    return m_position;
}

glm::vec3 Transformation::getRotation() const
{
    return m_rotation;
}

glm::vec3 Transformation::getForward() const
{
    glm::vec4 v = glm::yawPitchRoll(m_rotation.y, m_rotation.x, m_rotation.z)
        * glm::vec4(Constants::forward.x, Constants::forward.y, Constants::forward.z, 0.0f);
    return glm::vec3(v.x, v.y, v.z);
}

glm::vec3 Transformation::getUp() const
{
    glm::vec4 v = glm::yawPitchRoll(m_rotation.y, m_rotation.x, m_rotation.z)
        * glm::vec4(Constants::up.x, Constants::up.y, Constants::up.z, 0.0f);
    return glm::vec3(v.x, v.y, v.z);
}

glm::vec3 Transformation::getLeft() const
{
    glm::vec4 v = glm::yawPitchRoll(m_rotation.y, m_rotation.x, m_rotation.z)
        * glm::vec4(Constants::left.x, Constants::left.y, Constants::left.z, 0.0f);
    return glm::vec3(v.x, v.y, v.z);
}

const glm::mat4x4& Transformation::getWorldMatrix() const
{
    return m_worldMatrix;
}

void Transformation::updateWorldMatrix()
{
    m_worldMatrix = glm::translate(m_position) * glm::yawPitchRoll(m_rotation.y, m_rotation.x, m_rotation.z)
        * glm::scale(m_scale);
}

} // namespace fw
