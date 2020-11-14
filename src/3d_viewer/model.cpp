#include <glad/glad.h>

#include "model.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <iostream>

void ObjectModel::Draw(Shader &shader) const {
  // bind appropriate textures
  unsigned int diffuseNr = 1;
  unsigned int specularNr = 1;
  unsigned int normalNr = 1;
  unsigned int heightNr = 1;
  for (unsigned int i = 0; i < textures.size(); i++) {
    glActiveTexture(GL_TEXTURE0 + i);

    // bind texture unit to shader sampler variable
    //- the N in diffuse_textureN ...
    std::string N;
    std::string name = textures[i].type;
    if (name == "texture_diffuse")
      N = std::to_string(diffuseNr++);
    else if (name == "texture_specular")
      N = std::to_string(specularNr++);
    else if (name == "texture_normal")
      N = std::to_string(normalNr++);
    else if (name == "texture_height")
      N = std::to_string(heightNr++);
    //- now set the sampler to the correct texture unit
    glUniform1i(glGetUniformLocation(shader.ID, (name + N).c_str()), i);

    // bind the texture
    glBindTexture(GL_TEXTURE_2D, textures[i].id);
  }

  // choose drawing mode due to primitive type
  GLenum draw_mode;
  if (primitive_type == POINTS)
    draw_mode = GL_POINTS;
  else if (primitive_type == LINES)
    draw_mode = GL_LINES;
  else if (primitive_type == LINE_STRIP)
    draw_mode = GL_LINE_STRIP;
  else if (primitive_type == TRIANGLES)
    draw_mode = GL_TRIANGLES;
  else if (primitive_type == TRIANGLE_STRIP)
    draw_mode = GL_TRIANGLE_STRIP;
  else {
    std::cerr << "invalid primitive type: " << primitive_type << std::endl;
    return;
  }

  // draw mesh
  glBindVertexArray(VAO);
  if (indices.empty())
    glDrawArrays(draw_mode, 0, positions.size());
  else
    glDrawElements(draw_mode, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  // set everything back to defaults
  glActiveTexture(GL_TEXTURE0);
}

void ObjectModel::LoadIntoBuffers() {
  // compute buffer size
  unsigned int v_num = positions.size();
  unsigned int buf_size = v_num * sizeof(glm::vec3);
  if (normals.size() == v_num) buf_size += v_num * sizeof(glm::vec3);
  if (tex_coords.size() == v_num) buf_size += v_num * sizeof(glm::vec2);
  if (tangents.size() == v_num) buf_size += v_num * sizeof(glm::vec3);
  if (bitangents.size() == v_num) buf_size += v_num * sizeof(glm::vec3);
  if (colors.size() == v_num) buf_size += v_num * sizeof(glm::vec4);

  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  if (!indices.empty()) glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  // load data into vertex buffers and set the vertex attribute pointers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, buf_size, NULL, GL_STATIC_DRAW);
  GLintptr attr_pos = 0;
  //- positions
  glBufferSubData(GL_ARRAY_BUFFER, attr_pos, v_num * sizeof(glm::vec3),
                  positions.data());
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                        (void *)attr_pos);
  attr_pos += v_num * sizeof(glm::vec3);
  //- normals
  if (normals.size() == v_num) {
    glBufferSubData(GL_ARRAY_BUFFER, attr_pos, v_num * sizeof(glm::vec3),
                    normals.data());
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void *)attr_pos);
    attr_pos += v_num * sizeof(glm::vec3);
  }
  //- texture coordinates
  if (tex_coords.size() == v_num) {
    glBufferSubData(GL_ARRAY_BUFFER, attr_pos, v_num * sizeof(glm::vec2),
                    tex_coords.data());
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec2),
                          (void *)attr_pos);
    attr_pos += v_num * sizeof(glm::vec2);
  }
  //- tangent vectors
  if (tangents.size() == v_num) {
    glBufferSubData(GL_ARRAY_BUFFER, attr_pos, v_num * sizeof(glm::vec3),
                    tangents.data());
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void *)attr_pos);
    attr_pos += v_num * sizeof(glm::vec3);
  }
  //- bitangent vectors
  if (bitangents.size() == v_num) {
    glBufferSubData(GL_ARRAY_BUFFER, attr_pos, v_num * sizeof(glm::vec3),
                    bitangents.data());
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3),
                          (void *)attr_pos);
    attr_pos += v_num * sizeof(glm::vec3);
  }
  //- colors
  if (colors.size() == v_num) {
    glBufferSubData(GL_ARRAY_BUFFER, attr_pos, v_num * sizeof(glm::vec4),
                    colors.data());
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec4),
                          (void *)attr_pos);
    attr_pos += v_num * sizeof(glm::vec4);
  }
  // fill element buffer
  if (!indices.empty()) {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
                 &indices[0], GL_STATIC_DRAW);
  }

  // unbind VAO
  glBindVertexArray(0);
}

void ObjectModel::ReleaseBuffers() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  if (!indices.empty()) glDeleteBuffers(1, &EBO);
}

void Transform(std::vector<glm::vec3> &positions,
               std::vector<glm::vec3> &normals, const glm::mat4 &T) {
  unsigned int N = positions.size();
  for (unsigned int i = 0; i < N; ++i) {
    glm::mat3 R(T);
    normals[i] = R * normals[i];
    glm::vec4 p(positions[i], 1.0f);
    p = T * p;
    positions[i] = glm::vec3(p);
  }
}

void SceneModel::loadModel(std::string const &path) {
  // read file via ASSIMP
  Assimp::Importer importer;
  const aiScene *scene = importer.ReadFile(
      path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
  // check for errors
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE ||
      !scene->mRootNode) {
    std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
    return;
  }
  // retrieve the directory path of the filepath
  directory = path.substr(0, path.find_last_of('/'));

  // process ASSIMP's root node recursively
  processNode(scene->mRootNode, scene);
}

// processes a node in a recursive fashion.
void SceneModel::processNode(aiNode *node, const aiScene *scene) {
  // process each mesh located at the current node
  for (unsigned int i = 0; i < node->mNumMeshes; i++) {
    aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
    meshes.push_back(processMesh(mesh, scene));
  }
  // recursively process each of the children nodes
  for (unsigned int i = 0; i < node->mNumChildren; i++) {
    processNode(node->mChildren[i], scene);
  }
}

ObjectModel SceneModel::processMesh(aiMesh *mesh, const aiScene *scene) {
  // data to fill
  ObjectModel res;
  res.primitive_type = ObjectModel::TRIANGLES;

  // walk through each of the mesh's vertices
  res.positions.resize(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    res.positions[i].x = mesh->mVertices[i].x;
    res.positions[i].y = mesh->mVertices[i].y;
    res.positions[i].z = mesh->mVertices[i].z;
  }
  // normals
  if (mesh->HasNormals()) {
    res.normals.resize(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      res.normals[i].x = mesh->mNormals[i].x;
      res.normals[i].y = mesh->mNormals[i].y;
      res.normals[i].z = mesh->mNormals[i].z;
    }
  }
  // texture coordinates
  if (mesh->mTextureCoords[0]) {
    res.tex_coords.resize(mesh->mNumVertices);
    res.tangents.resize(mesh->mNumVertices);
    res.bitangents.resize(mesh->mNumVertices);
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
      // a vertex can contain up to 8 texture coordinates. only use 1.
      res.tex_coords[i].x = mesh->mTextureCoords[0][i].x;
      res.tex_coords[i].y = mesh->mTextureCoords[0][i].y;
      // tangent
      res.tangents[i].x = mesh->mTangents[i].x;
      res.tangents[i].y = mesh->mTangents[i].y;
      res.tangents[i].z = mesh->mTangents[i].z;
      // bitangent
      res.bitangents[i].x = mesh->mBitangents[i].x;
      res.bitangents[i].y = mesh->mBitangents[i].y;
      res.bitangents[i].z = mesh->mBitangents[i].z;
    }
  }
  // now wak through each of the mesh's faces (triangle)
  res.indices.reserve(3 * mesh->mNumFaces);
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      res.indices.push_back(face.mIndices[j]);
  }
  // process materials
  aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];

  // assume a convention for sampler names in the shaders. Each diffuse
  // texture should be named as 'texture_diffuseN' where N is a sequential
  // number ranging from 1 to MAX_SAMPLER_NUMBER. Same applies to other texture
  // such as : texture_specularN, texture_normalN

  // 1. diffuse maps
  std::vector<Texture> diffuseMaps =
      loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
  res.textures.insert(res.textures.end(), diffuseMaps.begin(),
                      diffuseMaps.end());
  // 2. specular maps
  std::vector<Texture> specularMaps = loadMaterialTextures(
      material, aiTextureType_SPECULAR, "texture_specular");
  res.textures.insert(res.textures.end(), specularMaps.begin(),
                      specularMaps.end());
  // 3. normal maps
  std::vector<Texture> normalMaps =
      loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
  res.textures.insert(res.textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<Texture> heightMaps =
      loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
  res.textures.insert(res.textures.end(), heightMaps.begin(), heightMaps.end());

  // load mesh data into GPU buffers
  res.LoadIntoBuffers();
  return res;
}

// checks all material textures of a given type and loads the textures if
// they're not loaded yet. return textures newly loaded.
std::vector<Texture> SceneModel::loadMaterialTextures(aiMaterial *mat,
                                                      aiTextureType type,
                                                      std::string typeName) {
  std::vector<Texture> textures;
  for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
    aiString str;
    mat->GetTexture(type, i, &str);
    // check if texture was loaded before
    bool skip = false;
    for (unsigned int j = 0; j < textures_loaded.size(); j++) {
      // compare file path with loaded textures
      if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0) {
        textures.push_back(textures_loaded[j]);
        skip = true;
        break;
      }
    }
    if (!skip) {
      Texture texture;
      texture.id = TextureFromFile(str.C_Str(), this->directory);
      texture.type = typeName;
      texture.path = str.C_Str();
      textures.push_back(texture);
      textures_loaded.push_back(texture);
    }
  }
  return textures;
}

// Load texture into GPU from file
unsigned int TextureFromFile(const char *path, const std::string &directory,
                             bool gamma) {
  std::string filename = std::string(path);
  filename = directory + '/' + filename;

  unsigned int textureID;
  glGenTextures(1, &textureID);

  stbi_set_flip_vertically_on_load(true);
  int width, height, nrComponents;
  unsigned char *data =
      stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
  if (data) {
    GLenum format;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
                 GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  } else {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}

void Texture::Release() { glDeleteTextures(1, &id); }

const ObjectModel &ObjectModel::UnitCube() {
  static ObjectModel cube;
  cube.primitive_type = ObjectModel::TRIANGLES;

  // clang-format off
  static float cube_vertices[] = {
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

  static bool first_run = true;
  if (first_run) {
    first_run = false;
    cube.positions.resize(36);
    cube.normals.resize(36);
    cube.tex_coords.resize(36);
    for (int i = 0; i < 36; ++i) {
      cube.positions[i].x = cube_vertices[8 * i + 0];
      cube.positions[i].y = cube_vertices[8 * i + 1];
      cube.positions[i].z = cube_vertices[8 * i + 2];
      cube.normals[i].x = cube_vertices[8 * i + 3];
      cube.normals[i].y = cube_vertices[8 * i + 4];
      cube.normals[i].z = cube_vertices[8 * i + 5];
      cube.tex_coords[i].x = cube_vertices[8 * i + 6];
      cube.tex_coords[i].y = cube_vertices[8 * i + 7];
    }
    cube.LoadIntoBuffers();
  }
  return cube;
}

const ObjectModel &ObjectModel::UnitQuad() {
  static ObjectModel quad;
  quad.primitive_type = ObjectModel::TRIANGLES;

  // clang-format off
  static float quad_vertices[] = {
       // positions         // normals          // texcoords
      -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  // bottom-left
       1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f,  // bottom-right
       1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,  // top-right
       1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f,  // top-right
      -1.0f,  1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f,  // top-left
      -1.0f, -1.0f,  0.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f,  // bottom-left
  };
  // clang-format on

  static bool first_run = true;
  if (first_run) {
    first_run = false;
    quad.positions.resize(6);
    quad.normals.resize(6);
    quad.tex_coords.resize(6);
    for (int i = 0; i < 6; ++i) {
      quad.positions[i].x = quad_vertices[8 * i + 0];
      quad.positions[i].y = quad_vertices[8 * i + 1];
      quad.positions[i].z = quad_vertices[8 * i + 2];
      quad.normals[i].x = quad_vertices[8 * i + 3];
      quad.normals[i].y = quad_vertices[8 * i + 4];
      quad.normals[i].z = quad_vertices[8 * i + 5];
      quad.tex_coords[i].x = quad_vertices[8 * i + 6];
      quad.tex_coords[i].y = quad_vertices[8 * i + 7];
    }
    quad.LoadIntoBuffers();
  }
  return quad;
}

ObjectModel ObjectModel::Quad(float r) {
  ObjectModel quad = ObjectModel::UnitQuad();
  for (auto &p : quad.positions) p *= r;
  for (auto &t : quad.tex_coords) t *= r;
  return quad;
}

TrackballModel::TrackballModel() : m_circle_discretization(32) {
  m_circle.primitive_type = ObjectModel::LINE_STRIP;
  m_circle.positions.resize(m_circle_discretization, glm::vec3(0.0f));
  m_circle.indices.resize(m_circle_discretization + 1);
  float theta = 2 * M_PI / m_circle_discretization;
  for (int i = 0; i < m_circle_discretization; ++i) {
    glm::vec3 &v = m_circle.positions[i];
    float angle = i * theta;
    v.x = std::cos(angle);
    v.y = std::sin(angle);
  }
  for (unsigned int i = 0; i < m_circle_discretization; ++i)
    m_circle.indices[i] = i;
  m_circle.indices[m_circle_discretization] = 0;
  m_circle.LoadIntoBuffers();
}

void TrackballModel::ReleaseBuffers() {
  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
}

void TrackballModel::Draw(Shader &shader, const glm::mat4 &model) const {
  glm::mat4 M;

  // xy plane
  shader.setVec4("Color", glm::vec4(0.0f, 0.0f, 1.0f, 1.0f));
  M = model;
  shader.setMat4("model", M);
  m_circle.Draw(shader);

  // yz plane
  shader.setVec4("Color", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
  M = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
  shader.setMat4("model", M);
  m_circle.Draw(shader);

  // zx plane
  shader.setVec4("Color", glm::vec4(0.0f, 1.0f, 0.0f, 1.0f));
  M = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
  shader.setMat4("model", M);
  m_circle.Draw(shader);
}
