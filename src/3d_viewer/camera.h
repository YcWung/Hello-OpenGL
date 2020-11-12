#ifndef _3D_VIEWER_CAMERA_H
#define _3D_VIEWER_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

inline glm::mat4 EulerToMat4(float yaw, float pitch, float roll) {
  glm::mat4 R1(1.0f), R2(1.0f), R3(1.0f);
  float c1 = cos(yaw), s1 = sin(yaw);
  float c2 = cos(pitch), s2 = sin(pitch);
  float c3 = cos(roll), s3 = sin(roll);
  R1[0][0] = c1;
  R1[0][2] = -s1;
  R1[2][0] = s1;
  R1[2][2] = c1;
  R2[1][1] = c2;
  R2[1][2] = s2;
  R2[2][1] = -s2;
  R2[2][2] = c2;
  R3[0][0] = c3;
  R3[0][1] = s3;
  R3[1][0] = -s3;
  R3[1][1] = c3;
  return R1 * R2 * R3;
}

class Camera {
 public:
  glm::mat4 m_view;
  float Zoom;
  float near_plane;
  float far_plane;

  Camera(glm::mat4 view = glm::mat4(1.0f))
      : m_view(view), Zoom(45.0f), near_plane(0.1f), far_plane(100.0f) {}

  glm::vec3 Position() const {
    glm::mat3 R(m_view);
    glm::vec3 p(m_view[3]);
    p = -glm::transpose(R) * p;
    return p;
  }
  void SetPosition(const glm::vec3 &p) {
    glm::mat3 R(m_view);
    m_view[3] = glm::vec4(-R * p, 0);
  }
  glm::vec3 Front() const {
    return glm::vec3(-m_view[0][2], -m_view[1][2], -m_view[2][2]);
  }

  void Reset(const glm::mat4 view) { m_view = view; }
  glm::mat4 GetViewMatrix() const { return m_view; }

  void translate(const glm::vec3 &t) { m_view[3] -= glm::vec4(t, 0.0f); }
  void rotate(float yaw, float pitch, float roll) {
    glm::mat4 R = EulerToMat4(yaw, pitch, roll);
    m_view = glm::transpose(R) * m_view;
  }
};

#endif  // _3D_VIEWER_CAMERA_H
