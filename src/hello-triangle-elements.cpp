#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

float vertices[] = {
  0.5f,  0.5f, 0.0f,  // top right
  0.5f, -0.5f, 0.0f,  // bottom right
  -0.5f, -0.5f, 0.0f,  // bottom left
  -0.5f,  0.5f, 0.0f   // top left 
};

unsigned int triangles[] = {
  0, 1, 3,  // first Triangle
  1, 2, 3   // second Triangle
};

const char *vertex_shader_source =
    "#version 330 core\n"
    "layout(location = 0) in vec3 a_pos;\n"
    "void main() {\n"
    "gl_Position = vec4(a_pos.x, a_pos.y, a_pos.z, 1.0);\n"
    "}\n";

const char *fragment_shader_source =
    "#version 330 core\n"
    "out vec4 frag_color;\n"
    "void main() { frag_color = vec4(1.0, 0.5, 0.2, 1.0); }\n";

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
  glViewport(0, 0, width, height);
}

void process_input(GLFWwindow* window);

int main(int argc, char **argv) {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // for MacOS
#endif

  GLFWwindow *window = glfwCreateWindow(800, 600, "Hello Triangle Elements", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window." << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  
  /* load OpenGL functions */
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD." << std::endl;
    return -1;
  }

  glViewport(0, 0, 800, 600);

  /* register callbacks */
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

  /* set up graphics pipeline */
  // vertex shader
  unsigned int vertex_shader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(vertex_shader, 1, &vertex_shader_source, NULL);
  glCompileShader(vertex_shader);
  int vs_compile_success;
  char vs_compile_log[512];
  glGetShaderiv(vertex_shader, GL_COMPILE_STATUS, &vs_compile_success);
  if (!vs_compile_success) {
    glGetShaderInfoLog(vertex_shader, 512, NULL, vs_compile_log);
    std::cerr << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << vs_compile_log
              << std::endl;
  }
  // fragment shader
  unsigned int fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(fragment_shader, 1, &fragment_shader_source, NULL);
  glCompileShader(fragment_shader);
  int fs_compile_success;
  char fs_compile_log[512];
  glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &fs_compile_success);
  if (!fs_compile_success) {
    glGetShaderInfoLog(fragment_shader, 512, NULL, fs_compile_log);
    std::cerr << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n"
              << fs_compile_log
              << std::endl;
  }
  // link shaders and activate program
  unsigned int shader_program = glCreateProgram();
  glAttachShader(shader_program, vertex_shader);
  glAttachShader(shader_program, fragment_shader);
  glLinkProgram(shader_program);
  int shader_link_success;
  char shader_link_log[512];
  glGetProgramiv(shader_program, GL_LINK_STATUS, &shader_link_success);
  if (!shader_link_success) {
    glGetProgramInfoLog(shader_program, 512, NULL, shader_link_log);
    std::cerr << "ERROR::PROGRAM::LINKING_FAILED\n" << shader_link_log << std::endl;
  }
  glUseProgram(shader_program);
  // delete linked shaders
  glDeleteShader(vertex_shader);
  glDeleteShader(fragment_shader);
  // create a VAO to store vertex attributes configuration
  unsigned int VAO;
  glGenVertexArrays(1, &VAO);
  glBindVertexArray(VAO);
  // vertex buffer
  unsigned int VBO;
  glGenBuffers(1, &VBO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
  // bind vertex attributes
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
  glEnableVertexAttribArray(0); // enable vertex attribute 0
  // element array buffer
  unsigned int EBO;
  glGenBuffers(1, &EBO);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(triangles), triangles, GL_STATIC_DRAW);
  // unbind buffers
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
  /* event loop */
  while(!glfwWindowShouldClose(window)) {
    // input
    process_input(window);

    //redering
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(shader_program);
    glBindVertexArray(VAO);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    
    // swap buffers and check events
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // de-allocate GPU resources
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteProgram(shader_program);

  glfwTerminate();
  
  return 0;
}

void process_input(GLFWwindow *window){
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);
}
