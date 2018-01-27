#include "CameraController.h"
#include "Common.h"
#include "Constants.h"
#include "API.h"

namespace fw
{

void CameraController::setCamera(Camera* c)
{
	camera = c;
}

void CameraController::setMovementSpeed(float s)
{
	movementSpeed = s;
}

void CameraController::setSensitivity(float s)
{
	sensitivity = s;
}

void CameraController::setResetMode(const glm::vec3& pos, const glm::vec3& rot, int key)
{
	resetPosition = pos;
	resetRotation = rot;
    resetKey = key;
}

void CameraController::update()
{
	if (!camera) {
		printWarning("Camera is not set for camera controller");
		return;
	}

	float speed = movementSpeed * API::getTimeDelta();
    const Transformation& t = camera->getTransformation();

	if (API::isKeyDown(GLFW_KEY_W)) {
		camera->move(t.getForward() * speed);
	}
	if (API::isKeyDown(GLFW_KEY_S)) {
		camera->move(-t.getForward() * speed);
	}
	if (API::isKeyDown(GLFW_KEY_A)) {
		camera->move(t.getLeft() * speed);
	}
	if (API::isKeyDown(GLFW_KEY_D)) {
		camera->move(-t.getLeft() * speed);
	}
	if (API::isKeyReleased(resetKey)) {
		camera->setPosition(resetPosition);
		camera->setRotation(resetRotation);
	}
    
	camera->rotate(Constants::up, API::getMouseDeltaX() * sensitivity);
	camera->rotate(Constants::left, -API::getMouseDeltaY() * sensitivity);

    glm::vec3 r = t.getRotation();
	if (r.x > Constants::rotationLimit) {
		r.x = Constants::rotationLimit;
	}
	if (r.x < -Constants::rotationLimit) {
		r.x = -Constants::rotationLimit;
    }
    camera->setRotation(r);
}

} // fw
