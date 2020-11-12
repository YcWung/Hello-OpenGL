#ifndef _3D_VIEWER_NAVIGATE_H
#define _3D_VIEWER_NAVIGATE_H

#include <iostream>

#include "camera.h"
#include "glm_util.h"

/** compute the rotation matrix sending u to v by axis cross(u, v)
 */
inline glm::mat4 VecToVecRotm4(const glm::vec3 &u, const glm::vec3 &v) {
  glm::mat4 R(1.0f);
  glm::vec3 axis = glm::cross(u, v);
  float sin_angle = norm(axis);
  if (sin_angle == 0.0f) return R;
  axis = axis / sin_angle;
  float angle = acos(glm::dot(u, v));
  R = glm::rotate(R, angle, axis);
  return R;
}

class Navigation {
 public:
  enum Camera_Movement { FORWARD, BACKWARD, LEFT, RIGHT };

  Navigation()
      : MovementSpeed(2.5f),
        MouseSensitivity(0.1f),
        m_trackball_center(0.0f, 0.0f, -1.0f) {}

  Camera &camera() { return m_camera; }

  /** tc is given in world coord. Rotate camera if necessary
   */
  void SetTrackballCenter(glm::vec3 tc) {
    glm::vec3 f1 = m_camera.Front();
    glm::vec3 f2 = tc - m_camera.Position();
    float d = norm(f2);
    f2 /= d;
    glm::mat4 R = VecToVecRotm4(f2, f1);
    m_camera.m_view = R * m_camera.m_view;
    m_trackball_center.z = -d;

    std::cout << "tc: " << tc << "\n";
    std::cout << "f1: " << f1 << "\n";
    std::cout << "f2: " << f2 << "\n";
    std::cout << "d: " << d << "\n";
    std::cout << "R:\n";
    for (int i = 0; i < 4; ++i) {
      for (int j = 0; j < 4; ++j) std::cout << "  " << R[j][i];
      std::cout << std::endl;
    }
  }

  glm::vec3 GetTrackballCenter() {
    glm::vec4 tc =
        glm::inverse(m_camera.m_view) * glm::vec4(m_trackball_center, 1.0f);
    return glm::vec3(tc);
  }
  float GetTrackballDistance() { return -m_trackball_center.z; }

  /** Move camera with trackball fixed in world space to fit a camera-trackball
   * distance.
   */
  void MoveCameraToTrackballDistance(float dis) {
    glm::vec3 shift(0.0f, 0.0f, dis + m_trackball_center.z);
    m_camera.translate(shift);
    m_trackball_center.z = -dis;
  }

  void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
    float velocity = MovementSpeed * deltaTime;
    glm::vec3 t(0.0f);
    if (direction == FORWARD) t.z = -velocity;
    if (direction == BACKWARD) t.z = velocity;
    if (direction == LEFT) t.x = -velocity;
    if (direction == RIGHT) t.x = velocity;
    m_camera.translate(t);
  }

  void ProcessMouseMovement(float xoffset, float yoffset) {
    float yaw = xoffset * MouseSensitivity / 180.0f * M_PI;
    float pitch = -yoffset * MouseSensitivity / 180.0f * M_PI;
    m_camera.translate(m_trackball_center);
    m_camera.rotate(yaw, pitch, 0.0f);
    m_camera.translate(-m_trackball_center);
  }

  void ProcessMouseScroll(float yoffset) {
    float zoom_ratio = 0.02f * yoffset;
    if (zoom_ratio > 0.1f) zoom_ratio = 0.1f;
    if (zoom_ratio < -0.1f) zoom_ratio = -0.1f;

    if ((-m_trackball_center.z == m_camera.near_plane && zoom_ratio > 0.0f) ||
        (-m_trackball_center.z == m_camera.far_plane && zoom_ratio < 0.0f)) {
      m_camera.Zoom *= (1.0f - zoom_ratio);
      if (m_camera.Zoom < 1.0f)
        m_camera.Zoom = 1.0f;
      else if (m_camera.Zoom > 45.0f)
        m_camera.Zoom = 45.0f;
    } else {
      float new_dis = -m_trackball_center.z * (1.0f - zoom_ratio);
      if (new_dis < m_camera.near_plane)
        new_dis = m_camera.near_plane;
      else if (new_dis > m_camera.far_plane)
        new_dis = m_camera.far_plane;
      MoveCameraToTrackballDistance(new_dis);
    }
  }

 private:
  Camera m_camera;
  glm::vec3 m_trackball_center;  // in camera coordinate system
  float MovementSpeed;
  float MouseSensitivity;
};

#endif  // _3D_VIEWER_NAVIGATE_H
