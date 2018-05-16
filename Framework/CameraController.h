#pragma once

#include "Camera.h"
#include <glm/glm.hpp>

namespace fw
{

class CameraController
{
public:
	CameraController() {};
	CameraController(const CameraController&) = delete;
	CameraController(CameraController&&) = delete;
	CameraController& operator=(const CameraController&) = delete;
	CameraController& operator=(CameraController&&) = delete;

	void setCamera(Camera* camera);
	void setMovementSpeed(float speed);
	void setSensitivity(float sensitivity);
	void setResetMode(const glm::vec3& pos, const glm::vec3& rot, int key);
	void update();

private:
	Camera* m_camera = nullptr;
	float m_movementSpeed = 5.0f;
	float m_sensitivity = 0.003f;
	glm::vec3 m_resetPosition;
	glm::vec3 m_resetRotation;
	int m_resetKey = -1;
};

} // fw
