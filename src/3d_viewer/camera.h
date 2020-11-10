#ifndef _3D_VIEWER_CAMERA_H
#define _3D_VIEWER_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

// default parameters
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
 public:
  glm::mat4 m_view;
  // camera options
  float MovementSpeed;
  float MouseSensitivity;
  float Zoom;

  Camera(glm::mat4 view = glm::mat4(1.0f))
      : m_view(view),
        MovementSpeed(SPEED),
        MouseSensitivity(SENSITIVITY),
        Zoom(ZOOM) {}

  glm::vec3 Position() {
    glm::mat3 R(m_view);
    glm::vec3 p(m_view[3]);
    p = glm::transpose(R) * p;
    return p;
  }
  void SetPosition(const glm::vec3 &p) {
    glm::mat3 R(m_view);
    m_view[3] = glm::vec4(-R * p, 0);
  }

  void Reset(const glm::mat4 view) { m_view = view; }
  glm::mat4 GetViewMatrix() const { return m_view; }

  void translate(const glm::vec3 &t) { m_view[3] -= glm::vec4(t, 0.0f); }

  void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    glm::vec3 t(0.0f);
    if (direction == FORWARD) t.z = -velocity;
    if (direction == BACKWARD) t.z = velocity;
    if (direction == LEFT) t.x = -velocity;
    if (direction == RIGHT) t.x = velocity;
    translate(t);
  }

  void ProcessMouseMovement(float xoffset, float yoffset,
                            bool constrainPitch = true) {
    float yaw = -xoffset * MouseSensitivity / 180.0f * M_PI;
    float pitch = yoffset * MouseSensitivity / 180.0f * M_PI;
    float c1 = cos(yaw), s1 = sin(yaw);
    float c2 = cos(pitch), s2 = sin(pitch);
    glm::mat4 R(1.0f);
    R[0] = {c1, 0.0f, -s1, 0.0f};
    R[1] = {s1 * s2, c2, c1 * s2, 0.0f};
    R[2] = {s1 * c2, -s2, c1 * c2, 0.0f};
    m_view = R * m_view;
  }

  void ProcessMouseScroll(float yoffset) {
    // change fov
    Zoom -= (float)yoffset;
    if (Zoom < 1.0f) Zoom = 1.0f;
    if (Zoom > 45.0f) Zoom = 45.0f;
  }
};

#endif  // _3D_VIEWER_CAMERA_H
