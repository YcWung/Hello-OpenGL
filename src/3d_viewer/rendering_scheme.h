#ifndef _3D_VIEWER_RENDERING_SCHEME_H
#define _3D_VIEWER_RENDERING_SCHEME_H

#include "model.h"
#include "navigate.h"
#include "shader.h"

class RenderingScheme {
 public:
  virtual void Render() = 0;
  void init(const Model*, Navigation*);
  void SetModel(const Model*);
  void SetNavigation(Navigation* nav) { m_navigation = nav; }
  void SetLight(const glm::vec3& p, const glm::vec3& dir, float near, float far,
                float r) {
    m_lightPos = p;
    m_lightDirection = dir;
    m_lightNearPlane = near;
    m_lightFarPlane = far;
    m_lightRadius = r;
  }

 protected:
  const Model* m_model;
  Navigation* m_navigation;

  float bbox[6];  // { xmin, xmax, ymin, ymax, zmin, zmax }
  glm::vec3 m_lightPos;
  glm::vec3 m_lightDirection;
  glm::vec3 m_bboxCenter;
  float m_lightNearPlane, m_lightFarPlane, m_lightRadius;
  unsigned int m_colorTexUnitNum;
};

class DirectionalLightingShadowScheme : public RenderingScheme {
 public:
  DirectionalLightingShadowScheme();
  DirectionalLightingShadowScheme(const Model*, Navigation*);
  ~DirectionalLightingShadowScheme();
  virtual void Render() override;

 private:
  Shader shader;
  Shader simpleDepthShader;
  Shader uniColorShader;
  unsigned int depthMapFBO;
  unsigned int depthMap;
  const unsigned int SHADOW_WIDTH, SHADOW_HEIGHT;
  TrackballModel m_trackball;
};

class SimpleRenderingScheme : public RenderingScheme {
 public:
  SimpleRenderingScheme();
  SimpleRenderingScheme(const Model*, Navigation*);
  virtual void Render() override;

 private:
  Shader shader;
};

#endif  // _3D_VIEWER_RENDERING_SCHEME_H
