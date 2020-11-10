#include <glad/glad.h>

#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <filesystem>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

#include "camera.h"
#include "config.h"
#include "model.h"
#include "rendering_scheme.h"
#include "shader.h"

#define SCR_WIDTH 800
#define SCR_HEIGHT 600

// event callbacks
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
// process keyboard inputs
void process_keyboard_input(GLFWwindow *window);

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool first_mouse = true;

// timing
float delta_time = 0.0f;
float last_frame = 0.0f;

GLFWwindow *InitWindowOpenGL() {
  // create window and OpenGL context
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
  GLFWwindow *window =
      glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Viewer", NULL, NULL);
  if (window == NULL) {
    std::cerr << "Failed to create OpenGL Context." << std::endl;
    glfwTerminate();
    return window;
  }
  glfwMakeContextCurrent(window);

  // set callbacks
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSetCursorPosCallback(window, cursor_pos_callback);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  glfwSetScrollCallback(window, scroll_callback);

  // load OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD" << std::endl;
    glfwTerminate();
    return NULL;
  }
  return window;
}

Model CreateTestModel();

int main(int argc, char **argv) {
  Config &config = Config::Instance();
  // resource path
  auto exe_dir = std::filesystem::absolute(argv[0]).parent_path();
  auto resource_path = exe_dir / "resources";
  config.exe_dir = exe_dir.string();
  config.resource_dir = resource_path.string();
  std::string model_file_path =
      (resource_path / "backpack" / "backpack.obj").string();

  // init GLFW window and OpenGL context
  GLFWwindow *window = InitWindowOpenGL();
  if (!window) return -1;

  /* set up data and rendering scheme */
  // Model model(model_file_path);
  Model model = CreateTestModel();
  DirectionalLightingShadowScheme rendering_scheme;
  rendering_scheme.SetModel(&model);
  rendering_scheme.SetCamera(&camera);
  rendering_scheme.SetLight(glm::vec3(-2.0f, 4.0f, -1.0f),
                            -glm::vec3(-2.0f, 4.0f, -1.0f), 1.0, 7.5, 10.0);
  // SimpleRenderingScheme rendering_scheme(&model, &camera);

  // event loop
  while (!glfwWindowShouldClose(window)) {
    // per frame time logic
    float current_frame = glfwGetTime();
    delta_time = current_frame - last_frame;
    last_frame = current_frame;
    // process keyboard inputs
    process_keyboard_input(window);

    /* render */
    rendering_scheme.Render();
    // swap buffers and poll events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // end
  glfwTerminate();
  return 0;
}

Model CreateTestModel() {
  // load the wood texture
  Texture wood_tex;
  wood_tex.id = TextureFromFile("wood.png", Config::Instance().resource_dir);
  wood_tex.type = "texture_diffuse";

  // create a mesh of standard cube
  Mesh cube;
  cube.textures.push_back(wood_tex);
  // clang-format off
  float cube_vertices[] = {
      // back face
      -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
       1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,  // top-right
       1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f,  // bottom-right
       1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f,  // top-right
      -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f,  // bottom-left
      -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f,  // top-left
      // front face                
      -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  // bottom-left
       1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  // bottom-right
       1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,  // top-right
       1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,  // top-right
      -1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,  // top-left
      -1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  // bottom-left
      // left face                 
      -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // top-right
      -1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f,  // top-left
      -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-left
      -1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-left
      -1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f,  // bottom-right
      -1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // top-right
      // right face                       
       1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // top-left
       1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right
       1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f,  // top-right
       1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f,  // bottom-right
       1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f,  // top-left
       1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f,  // bottom-left
      // bottom face                      
      -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  // top-right
       1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f,  // top-left
       1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  // bottom-left
       1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f,  // bottom-left
      -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f,  // bottom-right
      -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f,  // top-right
      // top face                         
      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  // top-left
       1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  // bottom-right
       1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f,  // top-right
       1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f,  // bottom-right
      -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f,  // top-left
      -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f   // bottom-left
  };
  // clang-format on
  cube.vertices.resize(36);
  cube.indices.resize(36);
  for (int i = 0; i < 36; ++i) {
    auto &v = cube.vertices[i];
    v.Position.x = cube_vertices[8 * i + 0];
    v.Position.y = cube_vertices[8 * i + 1];
    v.Position.z = cube_vertices[8 * i + 2];
    v.Normal.x = cube_vertices[8 * i + 3];
    v.Normal.y = cube_vertices[8 * i + 4];
    v.Normal.z = cube_vertices[8 * i + 5];
    v.TexCoords.x = cube_vertices[8 * i + 6];
    v.TexCoords.y = cube_vertices[8 * i + 7];

    cube.indices[i] = i;
  }

  // create model
  Model model;
  model.meshes.resize(4);
  model.textures_loaded.push_back(wood_tex);
  //- add cube 0
  model.meshes[0] = cube;
  glm::mat4 M;
  M = glm::mat4(1.0f);
  M = glm::translate(M, glm::vec3(0.0f, 1.5f, 0.0));
  M = glm::scale(M, glm::vec3(0.5f));
  Transform(model.meshes[0].vertices, M);
  model.meshes[0].LoadIntoBuffers();
  //- add cube 1
  model.meshes[1] = cube;
  M = glm::mat4(1.0f);
  M = glm::translate(M, glm::vec3(2.0f, 0.0f, 1.0));
  M = glm::scale(M, glm::vec3(0.5f));
  Transform(model.meshes[1].vertices, M);
  model.meshes[1].LoadIntoBuffers();
  //- add cube 2
  model.meshes[2] = cube;
  M = glm::mat4(1.0f);
  M = glm::translate(M, glm::vec3(-1.0f, 0.0f, 2.0));
  M = glm::scale(M, glm::vec3(0.5f));
  Transform(model.meshes[2].vertices, M);
  model.meshes[2].LoadIntoBuffers();
  //- add a plane
  Mesh &plane = model.meshes[3];
  plane.textures.push_back(wood_tex);
  // clang-format off
  float plane_vertices[] = {
      // positions            // normals         // texcoords
       25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
      -25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,   0.0f,  0.0f,
      -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,

       25.0f, -0.5f,  25.0f,  0.0f, 1.0f, 0.0f,  25.0f,  0.0f,
      -25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,   0.0f, 25.0f,
       25.0f, -0.5f, -25.0f,  0.0f, 1.0f, 0.0f,  25.0f, 25.0f
  };
  // clang-format on
  plane.vertices.resize(6);
  plane.indices.resize(6);
  for (int i = 0; i < 6; ++i) {
    auto &v = plane.vertices[i];
    v.Position.x = plane_vertices[8 * i + 0];
    v.Position.y = plane_vertices[8 * i + 1];
    v.Position.z = plane_vertices[8 * i + 2];
    v.Normal.x = plane_vertices[8 * i + 3];
    v.Normal.y = plane_vertices[8 * i + 4];
    v.Normal.z = plane_vertices[8 * i + 5];
    v.TexCoords.x = plane_vertices[8 * i + 6];
    v.TexCoords.y = plane_vertices[8 * i + 7];

    plane.indices[i] = i;
  }
  plane.LoadIntoBuffers();

  return model;
}

void process_keyboard_input(GLFWwindow *window) {
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  else if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    camera.ProcessKeyboard(FORWARD, delta_time);
  else if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    camera.ProcessKeyboard(BACKWARD, delta_time);
  else if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    camera.ProcessKeyboard(LEFT, delta_time);
  else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    camera.ProcessKeyboard(RIGHT, delta_time);
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow *window, double xpos, double ypos) {
  if (first_mouse) {
    lastX = xpos;
    lastY = ypos;
    first_mouse = false;
  }
  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos;
  lastX = xpos;
  lastY = ypos;
  camera.ProcessMouseMovement(xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
  camera.ProcessMouseScroll(yoffset);
}