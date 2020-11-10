#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>

#include <fstream>
#include <glm/glm.hpp>
#include <iostream>
#include <sstream>
#include <string>

class Shader {
 public:
  unsigned int ID;  // program id

  Shader(const char *vs_path, const char *fs_path) {
    // read source code
    std::string vs_code, fs_code;
    std::ifstream vs_ins, fs_ins;
    vs_ins.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fs_ins.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    try {
      vs_ins.open(vs_path);
      fs_ins.open(fs_path);
      std::stringstream vs_ss, fs_ss;
      vs_ss << vs_ins.rdbuf();
      fs_ss << fs_ins.rdbuf();
      vs_ins.close();
      fs_ins.close();
      vs_code = vs_ss.str();
      fs_code = fs_ss.str();
    } catch (std::ifstream::failure &e) {
      std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ" << std::endl;
    }
    // compile shaders
    unsigned int vs = glCreateShader(GL_VERTEX_SHADER);
    const char *vs_code_ = vs_code.c_str();
    glShaderSource(vs, 1, &vs_code_, NULL);
    glCompileShader(vs);
    checkCompileErrors(vs, "VERTEX");

    unsigned int fs = glCreateShader(GL_FRAGMENT_SHADER);
    const char *fs_code_ = fs_code.c_str();
    glShaderSource(fs, 1, &fs_code_, NULL);
    glCompileShader(fs);
    checkCompileErrors(fs, "FRAGMENT");

    // link shaders
    ID = glCreateProgram();
    glAttachShader(ID, vs);
    glAttachShader(ID, fs);
    glLinkProgram(ID);
    checkCompileErrors(ID, "PROGRAM");

    // delete compiled shaders
    glDeleteShader(vs);
    glDeleteShader(fs);
  }

  void use() const { glUseProgram(ID); }
  // --------------------------------------------------------------
  void setBool(const std::string &name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
  }
  void setInt(const std::string &name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
  }
  void setFloat(const std::string &name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
  }
  // --------------------------------------------------------------
  void setVec2(const std::string &name, float x, float y) const {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
  }
  void setVec2(const std::string &name, const glm::vec2 &value) const {
    glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  // ----------------------------------------------------------------------
  void setVec3(const std::string &name, float x, float y, float z) const {
    glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
  }
  void setVec3(const std::string &name, const glm::vec3 &value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  // --------------------------------------------------------------------------------
  void setVec4(const std::string &name, float x, float y, float z,
               float w) const {
    glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w);
  }
  void setVec4(const std::string &name, const glm::vec4 &value) const {
    glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
  }
  // -----------------------------------------------------------------
  void setMat2(const std::string &name, const glm::mat2 &mat) const {
    glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }
  void setMat3(const std::string &name, const glm::mat3 &mat) const {
    glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }
  void setMat4(const std::string &name, const glm::mat4 &mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE,
                       &mat[0][0]);
  }

 private:
  void checkCompileErrors(unsigned int shader, std::string type) {
    GLint success;
    GLchar info_log[1024];
    if (type != "PROGRAM") {
      glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(shader, 1024, NULL, info_log);
        std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type
                  << std::endl
                  << info_log << std::endl;
      }
    } else {
      glGetProgramiv(shader, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(shader, 1024, NULL, info_log);
        std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type
                  << std::endl
                  << info_log << std::endl;
      }
    }
  }
};

#endif
