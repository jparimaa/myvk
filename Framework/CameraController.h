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

	void setCamera(Camera* c);
	void setMovementSpeed(float s);
	void setSensitivity(float s);
	void setResetMode(const glm::vec3& pos, const glm::vec3& rot, int key);
	void update();
	
private:
	Camera* camera = nullptr;
	float movementSpeed = 5.0f;
	float sensitivity = 1.0f;
	glm::vec3 resetPosition;
	glm::vec3 resetRotation;
	int resetKey = -1;
};

} // fw
