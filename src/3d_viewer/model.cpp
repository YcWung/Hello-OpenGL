#include <glad/glad.h>

#include "model.h"

#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <stb_image.h>

#include <assimp/Importer.hpp>
#include <iostream>

void Mesh::Draw(Shader &shader) const {
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

  // draw mesh
  glBindVertexArray(VAO);
  glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);

  // set everything back to defaults
  glActiveTexture(GL_TEXTURE0);
}

void Mesh::LoadIntoBuffers() {
  // create buffers/arrays
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);

  glBindVertexArray(VAO);
  // load data into vertex buffers
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0],
               GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int),
               &indices[0], GL_STATIC_DRAW);

  // set the vertex attribute pointers
  //- vertex Positions
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
  //- vertex normals
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Normal));
  //- vertex texture coords
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, TexCoords));
  //- vertex tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Tangent));
  //- vertex bitangent
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, Bitangent));

  glBindVertexArray(0);
}

void Transform(std::vector<Mesh::Vertex> &vertices, const glm::mat4 &T) {
  for (Mesh::Vertex &v : vertices) {
    glm::mat3 R(T);
    v.Normal = R * v.Normal;
    glm::vec4 p(v.Position, 1.0f);
    p = T * p;
    v.Position = glm::vec3(p);
  }
}

void Model::loadModel(std::string const &path) {
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
void Model::processNode(aiNode *node, const aiScene *scene) {
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

Mesh Model::processMesh(aiMesh *mesh, const aiScene *scene) {
  // data to fill
  Mesh res_mesh;
  std::vector<Mesh::Vertex> &vertices = res_mesh.vertices;
  std::vector<unsigned int> &indices = res_mesh.indices;
  std::vector<Texture> &textures = res_mesh.textures;

  // walk through each of the mesh's vertices
  vertices.resize(mesh->mNumVertices);
  for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
    Mesh::Vertex &vertex = vertices[i];
    // positions
    vertex.Position.x = mesh->mVertices[i].x;
    vertex.Position.y = mesh->mVertices[i].y;
    vertex.Position.z = mesh->mVertices[i].z;
    // normals
    if (mesh->HasNormals()) {
      vertex.Normal.x = mesh->mNormals[i].x;
      vertex.Normal.y = mesh->mNormals[i].y;
      vertex.Normal.z = mesh->mNormals[i].z;
    }
    // texture coordinates
    if (mesh->mTextureCoords[0])  // does the mesh contain texture coordinates?
    {
      glm::vec2 vec;
      // a vertex can contain up to 8 texture coordinates. only use 1.
      vertex.TexCoords.x = mesh->mTextureCoords[0][i].x;
      vertex.TexCoords.y = mesh->mTextureCoords[0][i].y;
      // tangent
      vertex.Tangent.x = mesh->mTangents[i].x;
      vertex.Tangent.y = mesh->mTangents[i].y;
      vertex.Tangent.z = mesh->mTangents[i].z;
      // bitangent
      vertex.Bitangent.x = mesh->mBitangents[i].x;
      vertex.Bitangent.y = mesh->mBitangents[i].y;
      vertex.Bitangent.z = mesh->mBitangents[i].z;
    } else
      vertex.TexCoords = glm::vec2(0.0f, 0.0f);
  }
  // now wak through each of the mesh's faces (triangle)
  indices.reserve(3 * mesh->mNumFaces);
  for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
    aiFace face = mesh->mFaces[i];
    for (unsigned int j = 0; j < face.mNumIndices; j++)
      indices.push_back(face.mIndices[j]);
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
  textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
  // 2. specular maps
  std::vector<Texture> specularMaps = loadMaterialTextures(
      material, aiTextureType_SPECULAR, "texture_specular");
  textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
  // 3. normal maps
  std::vector<Texture> normalMaps =
      loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
  textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
  // 4. height maps
  std::vector<Texture> heightMaps =
      loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
  textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

  // load mesh data into GPU buffers
  res_mesh.LoadIntoBuffers();
  return res_mesh;
}

// checks all material textures of a given type and loads the textures if
// they're not loaded yet. return textures newly loaded.
std::vector<Texture> Model::loadMaterialTextures(aiMaterial *mat,
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
