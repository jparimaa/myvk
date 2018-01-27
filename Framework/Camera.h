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
    void setRotation(const glm::vec3& rot);
	void move(const glm::vec3& translation);
	void rotate(const glm::vec3& axis, float amount);

    const Transformation& getTransformation() const;

    const glm::mat4x4& getViewMatrix() const;
    const glm::mat4x4& getProjectionMatrix() const;

private:
    float FOV = 45.0f;
	float ratio = 1.33333f;
	float nearClipDistance = 0.1f;
	float farClipDistance = 100.0f;

	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

    Transformation transformation;

    void updateViewMatrix();
    void updateProjectionMatrix();

};

} // namespace fw
