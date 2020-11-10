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
Camera camera;
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
  camera.translate(glm::vec3(0.0f, 0.0f, 3.0f));
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
  Mesh cube = Mesh::UnitCube();
  cube.textures.push_back(wood_tex);

  // Create model
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
  plane = Mesh::Quad(25.0f);
  M = glm::mat4(1.0f);
  M = glm::translate(M, glm::vec3(0.0f, -0.5f, 0.0f));
  M = glm::rotate(M, glm::radians(-90.0f), glm::vec3(1, 0, 0));
  Transform(plane.vertices, M);
  plane.textures.push_back(wood_tex);
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
