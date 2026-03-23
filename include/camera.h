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

    float fov = 110;
    float aspect;
    float nearPlane = 0.2f;
    float farPlane = 5000.0f;

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
    mat4 getProjectionMatrix();
    void setAspectRatio(float width, float height);
};
