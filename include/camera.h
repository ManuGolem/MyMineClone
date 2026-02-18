#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;
class Camera {
  private:
    vec3 position;
    vec3 front;
    vec3 frontHorizontal;
    vec3 up;
    vec3 right;
    vec3 worldUp;
    float yaw;
    float pitch;

  public:
    Camera();
    void updateCameraVectors();
    void moveForward(float);
    void moveBackward(float);
    void moveLeft(float);
    void moveRight(float);
    void moveUp(float);
    void moveDown(float);
    void processMouse(float, float);
    mat4 getViewMatrix();
    vec3 getFront() const {
        return front;
    }
    vec3 getPosition();
};
