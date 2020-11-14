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

  void Release();
};
unsigned int TextureFromFile(const char *path, const std::string &directory,
                             bool gamma = false);

class ObjectModel {
 public:
  enum PrimitiveType { POINTS, LINES, LINE_STRIP, TRIANGLES, TRIANGLE_STRIP };

  std::vector<glm::vec3> positions;
  std::vector<glm::vec3> normals;
  std::vector<glm::vec2> tex_coords;
  std::vector<glm::vec3> tangents;
  std::vector<glm::vec3> bitangents;
  std::vector<glm::vec4> colors;
  std::vector<unsigned int> indices;
  std::vector<Texture> textures;
  PrimitiveType primitive_type;

  // constructor
  ObjectModel() {}
  // ~Mesh();
  void ReleaseBuffers();

  // one-pass render
  void Draw(Shader &shader) const;
  // Load vertices data into buffers
  void LoadIntoBuffers();

  // Create standard shapes
  //- standard cube [(-1, -1, -1), (1, 1, 1)]
  static const ObjectModel &UnitCube();
  //- standard quad [(-1, -1, 0), (1, 1, 0)]
  static const ObjectModel &UnitQuad();
  // - quad [(-r, -r, 0), (r, r, 0)] with tex coords [(0, 0),(r, r)]
  static ObjectModel Quad(float r);

 private:
  unsigned int VAO;
  unsigned int VBO, EBO;
};

void Transform(std::vector<glm::vec3> &positions,
               std::vector<glm::vec3> &normals, const glm::mat4 &T);

class SceneModel {
 public:
  std::vector<Texture> textures_loaded;
  std::vector<ObjectModel> meshes;

  SceneModel() : gammaCorrection(false) {}
  // constructor, expects a filepath to a 3D model.
  SceneModel(std::string const &path, bool gamma = false)
      : gammaCorrection(gamma) {
    loadModel(path);
  }

  // draws the model, and thus all its meshes
  void Draw(Shader &shader) const {
    for (unsigned int i = 0; i < meshes.size(); i++) meshes[i].Draw(shader);
  }

  void ReleaseBuffers() {
    for (ObjectModel &mesh : meshes) mesh.ReleaseBuffers();
    for (Texture &tex : textures_loaded) tex.Release();
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
  ObjectModel processMesh(aiMesh *mesh, const aiScene *scene);
  /** checks all material textures of a given type and loads the textures if
   * they're not loaded yet. the required info is returned as a Texture struct.
   */
  std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type,
                                            std::string typeName);
};

class TrackballModel {
 public:
  TrackballModel();
  void ReleaseBuffers();
  void Draw(Shader &shader, const glm::mat4 &model) const;

 private:
  unsigned int VAO;
  unsigned int VBO, EBO;
  unsigned int m_circle_discretization;
  ObjectModel m_circle;
};

#endif  // _3D_VIEWER_MODEL_H
