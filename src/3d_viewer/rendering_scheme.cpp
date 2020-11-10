#include <glad/glad.h>

#include "rendering_scheme.h"

#include <iostream>

#include "config.h"

inline const std::string& res_dir() { return Config::Instance().exe_dir; }

DirectionalLightingShadowScheme::DirectionalLightingShadowScheme()
    : shader((res_dir() + "/shadow_rendering.vs").c_str(),
             (res_dir() + "/shadow_rendering.fs").c_str()),
      simpleDepthShader((res_dir() + "/depth_mapping.vs").c_str(),
                        (res_dir() + "/depth_mapping.fs").c_str()),
      SHADOW_WIDTH(1024),
      SHADOW_HEIGHT(1024) {
  // configure depth map FBO
  glGenFramebuffers(1, &depthMapFBO);
  // create depth texture
  glGenTextures(1, &depthMap);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH,
               SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
  float borderColor[] = {1.0, 1.0, 1.0, 1.0};
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
  // attach depth texture as FBO's depth buffer
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                         depthMap, 0);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

DirectionalLightingShadowScheme::DirectionalLightingShadowScheme(
    const Model* model, Camera* camera)
    : DirectionalLightingShadowScheme() {
  init(model, camera);
}

void DirectionalLightingShadowScheme::Render() {
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // save viewport dimension
  GLint viewport_old[4];
  glGetIntegerv(GL_VIEWPORT, viewport_old);
  GLint& SCR_WIDTH = viewport_old[2];
  GLint& SCR_HEIGHT = viewport_old[3];

  // std::cout << "Old Viewport: ";
  // for (int i = 0; i < 4; ++i) std::cout << " " << viewport_old[i];
  // std::cout << std::endl;

  // 1. render depth of scene to texture (from light's perspective)
  // --------------------------------------------------------------
  glm::mat4 lightProjection, lightView;
  glm::mat4 lightSpaceMatrix;
  lightProjection =
      glm::ortho(-m_lightRadius, m_lightRadius, -m_lightRadius, m_lightRadius,
                 m_lightNearPlane, m_lightFarPlane);
  lightView = glm::lookAt(m_lightPos, m_lightPos + m_lightDirection,
                          glm::vec3(0.0, 1.0, 0.0));
  lightSpaceMatrix = lightProjection * lightView;
  // render scene from light's point of view
  simpleDepthShader.use();
  simpleDepthShader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
  simpleDepthShader.setMat4("model", glm::mat4(1.0f));

  glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
  glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
  glClear(GL_DEPTH_BUFFER_BIT);
  m_model->Draw(simpleDepthShader);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  // reset viewport
  glViewport(viewport_old[0], viewport_old[1], viewport_old[2],
             viewport_old[3]);

  // 2. render scene as normal using the generated depth/shadow map
  // --------------------------------------------------------------
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  shader.use();
  glm::mat4 projection =
      glm::perspective(glm::radians(m_camera->Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
  glm::mat4 view = m_camera->GetViewMatrix();
  shader.setMat4("projection", projection);
  shader.setMat4("view", view);
  shader.setMat4("model", glm::mat4(1.0f));
  // set light uniforms
  shader.setVec3("viewPos", m_camera->Position);
  shader.setVec3("lightPos", m_lightPos);
  shader.setMat4("lightSpaceMatrix", lightSpaceMatrix);
  glActiveTexture(GL_TEXTURE0 + m_colorTexUnitNum);
  glBindTexture(GL_TEXTURE_2D, depthMap);
  shader.setInt("shadowMap", m_colorTexUnitNum);
  m_model->Draw(shader);
}

void RenderingScheme::SetModel(const Model* model) {
  m_model = model;
  // compute bounding box
  const glm::vec3& p0 = m_model->meshes[0].vertices[0].Position;
  bbox[0] = bbox[1] = p0.x;
  bbox[2] = bbox[3] = p0.y;
  bbox[4] = bbox[5] = p0.z;
  for (const Mesh& mesh : m_model->meshes) {
    for (const auto& vertex : mesh.vertices) {
      const glm::vec3& p = vertex.Position;
      if (p.x < bbox[0])
        bbox[0] = p.x;
      else if (p.x > bbox[1])
        bbox[1] = p.x;
      if (p.y < bbox[2])
        bbox[2] = p.y;
      else if (p.y > bbox[3])
        bbox[3] = p.y;
      if (p.z < bbox[4])
        bbox[4] = p.z;
      else if (p.z > bbox[5])
        bbox[5] = p.z;
    }
  }

  std::cout << "BBox: ";
  for (int i = 0; i < 6; ++i) std::cout << "  " << bbox[i];
  std::cout << std::endl;

  // Put a light
  glm::vec3 p1(bbox[0], bbox[2], bbox[4]);
  glm::vec3 p2(bbox[1], bbox[3], bbox[5]);
  m_lightPos = 3.0f * p2 + (1 - 3.0f) * p1;
  m_lightNearPlane = (m_lightPos - p2).length();
  m_lightFarPlane = (m_lightPos - p1).length();
  m_lightRadius = 0.51 * (p2 - p1).length();
  m_bboxCenter = 0.5f * p2 + 0.5f * p1;
  m_lightDirection = m_bboxCenter - m_lightPos;

  // compute number of texture units used by model.Draw
  unsigned int tex_num = 1;
  for (const Mesh& mesh : m_model->meshes)
    if (tex_num < mesh.textures.size()) tex_num = mesh.textures.size();
  m_colorTexUnitNum = tex_num;

  std::cout << "color texture units number = " << m_colorTexUnitNum
            << std::endl;
}

void RenderingScheme::init(const Model* model, Camera* camera) {
  SetModel(model);
  SetCamera(camera);
  m_camera->Position = m_bboxCenter;
  m_camera->Position.z +=
      bbox[1] - bbox[0] + bbox[3] - bbox[2] + bbox[5] - bbox[4];
  m_camera->Front = {0, 0, -1};
  m_camera->Up = {0, 1, 0};
}

SimpleRenderingScheme::SimpleRenderingScheme()
    : shader((res_dir() + "/simple_rendering.vs").c_str(),
             (res_dir() + "/simple_rendering.fs").c_str()) {}

SimpleRenderingScheme::SimpleRenderingScheme(const Model* model, Camera* camera)
    : SimpleRenderingScheme() {
  init(model, camera);
}

void SimpleRenderingScheme::Render() {
  glEnable(GL_DEPTH_TEST);
  glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  // viewport dimension
  GLint viewport[4];
  glGetIntegerv(GL_VIEWPORT, viewport);
  GLint& SCR_WIDTH = viewport[2];
  GLint& SCR_HEIGHT = viewport[3];
  glm::mat4 projection =
      glm::perspective(glm::radians(m_camera->Zoom),
                       (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
  glm::mat4 view = m_camera->GetViewMatrix();
  shader.use();
  shader.setMat4("projection", projection);
  shader.setMat4("view", view);
  shader.setMat4("model", glm::mat4(1.0f));
  m_model->Draw(shader);
}
