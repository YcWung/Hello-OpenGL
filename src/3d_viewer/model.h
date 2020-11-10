#ifndef _3D_VIEWER_MODEL_H
#define _3D_VIEWER_MODEL_H

#include <assimp/scene.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <vector>

#include "shader.h"

/** info of texture loaded into GPU memory
 */
struct Texture {
  unsigned int id;
  std::string type;
  std::string path;
};
unsigned int TextureFromFile(const char *path, const std::string &directory,
                             bool gamma = false);

class Mesh {
 public:
  struct Vertex {
    // position
    glm::vec3 Position;
    // normal
    glm::vec3 Normal;
    // texCoords
    glm::vec2 TexCoords;
    // tangent
    glm::vec3 Tangent;
    // bitangent
    glm::vec3 Bitangent;
  };

  std::vector<Vertex> vertices;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;

  // constructor
  Mesh() {}
  Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices,
       std::vector<Texture> textures)
      : vertices(vertices), indices(indices), textures(textures) {
    LoadIntoBuffers();
  }

  // one-pass render
  void Draw(Shader &shader) const;
  // Load vertices data into buffers
  void LoadIntoBuffers();

 private:
  unsigned int VAO;
  unsigned int VBO, EBO;
};

void Transform(std::vector<Mesh::Vertex> &vertices, const glm::mat4 &T);

class Model {
 public:
  std::vector<Texture> textures_loaded;
  std::vector<Mesh> meshes;

  Model() : gammaCorrection(false) {}
  // constructor, expects a filepath to a 3D model.
  Model(std::string const &path, bool gamma = false) : gammaCorrection(gamma) {
    loadModel(path);
  }

  // draws the model, and thus all its meshes
  void Draw(Shader &shader) const {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].Draw(shader);
  }

 private:
  std::string directory;
  bool gammaCorrection;
  /** loads a model with supported ASSIMP extensions from file and stores the
   * resulting meshes in the meshes vector.
   */
  void loadModel(std::string const &path);
  /** processes a node in a recursive fashion. Processes each individual mesh
   * located at the node and repeats this process on its children nodes (if
   * any).
   */
  void processNode(aiNode *node, const aiScene *scene);
  Mesh processMesh(aiMesh *mesh, const aiScene *scene);
  /** checks all material textures of a given type and loads the textures if
   * they're not loaded yet. the required info is returned as a Texture struct.
   */
  std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                            std::string typeName);
};

#endif  // _3D_VIEWER_MODEL_H
