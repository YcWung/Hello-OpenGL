#ifndef _3D_VIEWER_GLM_UTIL_H
#define _3D_VIEWER_GLM_UTIL_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

template <glm::length_t C, glm::length_t R, typename T,
          glm::qualifier Q = glm::defaultp>
std::ostream &operator<<(std::ostream &o, const glm::mat<C, R, T, Q> &m) {
  for (int i = 0; i < R; ++i) {
    for (int j = 0; j < C; ++j) o << "  " << m[j][i];
    o << std::endl;
  }
  return o;
}

template <glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
std::ostream &operator<<(std::ostream &o, const glm::vec<L, T, Q> &v) {
  o << v[0];
  for (int i = 1; i < L; ++i) o << "  " << v[i];
  return o;
}

template <glm::length_t L, typename T, glm::qualifier Q = glm::defaultp>
T norm(const glm::vec<L, T, Q> &v) {
  return std::sqrt(glm::dot(v, v));
}

#endif  // _3D_VIEWER_GLM_UTIL_H
