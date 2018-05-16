#include "CameraController.h"
#include "Common.h"
#include "Constants.h"
#include "API.h"

namespace fw
{

void CameraController::setCamera(Camera* camera)
{
	m_camera = camera;
}

void CameraController::setMovementSpeed(float speed)
{
	m_movementSpeed = speed;
}

void CameraController::setSensitivity(float sensitivity)
{
	m_sensitivity = sensitivity;
}

void CameraController::setResetMode(const glm::vec3& pos, const glm::vec3& rot, int key)
{
	m_resetPosition = pos;
	m_resetRotation = rot;
    m_resetKey = key;
}

void CameraController::update()
{
	if (!m_camera) {
		printWarning("Camera is not set for m_camera controller");
		return;
	}

	float speed = m_movementSpeed * API::getTimeDelta();
    const Transformation& t = m_camera->getTransformation();

	if (API::isKeyDown(GLFW_KEY_W)) {
		m_camera->move(t.getForward() * speed);
	}
	if (API::isKeyDown(GLFW_KEY_S)) {
		m_camera->move(-t.getForward() * speed);
	}
	if (API::isKeyDown(GLFW_KEY_A)) {
		m_camera->move(t.getLeft() * speed);
	}
	if (API::isKeyDown(GLFW_KEY_D)) {
		m_camera->move(-t.getLeft() * speed);
	}
	if (API::isKeyReleased(m_resetKey)) {
		m_camera->setPosition(m_resetPosition);
		m_camera->setRotation(m_resetRotation);
	}

	m_camera->rotate(Constants::up, API::getMouseDeltaX() * m_sensitivity);
	m_camera->rotate(Constants::left, -API::getMouseDeltaY() * m_sensitivity);

    glm::vec3 r = t.getRotation();
	if (r.x > Constants::rotationLimit) {
		r.x = Constants::rotationLimit;
	}
	if (r.x < -Constants::rotationLimit) {
		r.x = -Constants::rotationLimit;
    }
    m_camera->setRotation(r);
}

} // fw
