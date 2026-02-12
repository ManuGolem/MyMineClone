#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
private:
  glm::vec3 position;
  glm::vec3 front;
  glm::vec3 frontHorizontal;
  glm::vec3 up;
  glm::vec3 right;
  glm::vec3 worldUp;
  float yaw;
  float pitch;

public:
  Camera(glm::vec3 startPosition = glm::vec3(0.0f, 0.0f, 3.0f)) {
    position = startPosition;
    worldUp = glm::vec3(0.0f, 1.0f, 0.0f);
    yaw = 0.0f;
    pitch = 0.0f;
    updateCameraVectors();
  }
  void updateCameraVectors() {
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
  void moveForward(float amount) { position += frontHorizontal * amount; }

  void moveBackward(float amount) { position -= frontHorizontal * amount; }

  void moveLeft(float amount) { position -= right * amount; }

  void moveRight(float amount) { position += right * amount; }

  void moveUp(float amount) { position += worldUp * amount; }

  void moveDown(float amount) { position -= worldUp * amount; }
  void processMouse(float xoffset, float yoffset, bool constrainPitch = true) {
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
  // Getter para la matriz de vista
  glm::mat4 getViewMatrix() {
    return glm::lookAt(position, position + front, up);
  }

  // Getter para depuración
  glm::vec3 getPosition() { return position; }
};
