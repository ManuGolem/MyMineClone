#include "../include/camera.h"

Camera::Camera() {
    position = vec3(0.0f, 0.0f, 3.0f);
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = 0.0f;
    pitch = 0.0f;
    updateCameraVectors();
}
void Camera::updateCameraVectors() {
    // esta funcion no la entiendo mucho
    // copi pastee de internet
    // calcula el nuevo vector front
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    glm::vec3 newFrontH;
    newFrontH.x = cos(glm::radians(yaw));
    newFrontH.y = 0.0f; // <-- ¡QUITAMOS LA Y!
    newFrontH.z = sin(glm::radians(yaw));
    frontHorizontal = glm::normalize(newFrontH);
    // Recalcular right y up
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}
void Camera::moveForward(float amount) {
    position += frontHorizontal * amount;
}
void Camera::moveBackward(float amount) {
    position -= frontHorizontal * amount;
}
void Camera::moveLeft(float amount) {
    position -= right * amount;
}

void Camera::moveRight(float amount) {
    position += right * amount;
}

void Camera::moveUp(float amount) {
    position += worldUp * amount;
}

void Camera::moveDown(float amount) {
    position -= worldUp * amount;
}
void Camera::processMouse(float xoffset, float yoffset) {
    bool constrainPitch = true;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    // Actualizar ángulos
    yaw += xoffset;
    pitch += yoffset;

    // Evitar que la cámara se voltee
    if (constrainPitch) {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    // Actualizar vectores con los nuevos ángulos
    updateCameraVectors();
}
glm::mat4 Camera::getViewMatrix() {
    return glm::lookAt(position, position + front, up);
}

// Getter para depuración
glm::vec3 Camera::getPosition() {
    return position;
}
