#include "Model.hpp"

#include <filesystem>

unsigned int TextureFromFile(const char *path, const std::string &directory)
{
    std::filesystem::path filename = std::filesystem::path(directory) / std::filesystem::path(path);

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.string().c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = 0;
        if (nrComponents == 1) { format = GL_RED; }
        else if (nrComponents == 3) { format = GL_RGB; }
        else if (nrComponents == 4) { format = GL_RGBA; }

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
    }

    stbi_image_free(data);

    return textureID;
}

Model::Model(const std::string& filePath)
{
    loadModel(filePath);
}

void Model::render(const Shader& shader)
{
    for (const Mesh& mesh : _meshes)
    {
        mesh.render(shader);
    }
}

void Model::loadModel(const std::string& filePath)
{
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(
        filePath,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_FlipUVs |
        aiProcess_JoinIdenticalVertices |
        aiProcess_CalcTangentSpace
    );

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }

    // retrieve the directory path of the filepath
    _directory = std::filesystem::path(filePath).parent_path().string();

    // process ASSIMP's root node recursively
    processNode(scene->mRootNode, scene);
}

void Model::processNode(aiNode* node, const aiScene* scene)
{
    // process each mesh located at the current node
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene.
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        _meshes.push_back(processMesh(mesh, scene));
    }

    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

Mesh Model::processMesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices;
    vertices.reserve(mesh->mNumVertices);

    std::vector<unsigned int> indices;
    indices.reserve(mesh->mNumFaces * 3);

    std::vector<Texture> textures;

    // read vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;

        const auto& meshVertex = mesh->mVertices[i];
        vertex.Position = glm::vec3(meshVertex.x, meshVertex.y, meshVertex.z);

        if (mesh->HasNormals())
        {
            const auto& meshNormal = mesh->mNormals[i];
            vertex.Normal = glm::vec3(meshNormal.x, meshNormal.y, meshNormal.z);
        }

        // a vertex can contain up to 8 different texture coordinates.
        // We thus make the assumption that we won't
        // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
        if (mesh->mTextureCoords[0])
        {
            const auto& meshTexCoords = mesh->mTextureCoords[0][i];
            vertex.TexCoords = glm::vec2(meshTexCoords.x, meshTexCoords.y);

            if (mesh->mTangents)
            {
                const auto& meshTangent = mesh->mTangents[i];
                vertex.Tangent = glm::vec3(meshTangent.x, meshTangent.y, meshTangent.z);
            }

            if (mesh->mBitangents)
            {
                const auto& meshBitangent = mesh->mBitangents[i];
                vertex.Bitangent = glm::vec3(meshBitangent.x, meshBitangent.y, meshBitangent.z);
            }
        }
        else
        {
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        }

        vertices.push_back(vertex);
    }

    // read faces
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        const aiFace& face = mesh->mFaces[i];

        for (unsigned int j = 0; j < face.mNumIndices; j++)
        {
            indices.push_back(face.mIndices[j]);
        }
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

    // 2. specular maps
    std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());

    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());

    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    return Mesh(vertices, indices, textures);
}

std::vector<Texture> Model::loadMaterialTextures(aiMaterial* material, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;

    for (unsigned int i = 0; i < material->GetTextureCount(type); i++)
    {
        aiString str;
        material->GetTexture(type, i, &str);

        // check if texture was loaded before
        bool skip = false;
        for (unsigned int j = 0; j < _loadedTextures.size(); j++)
        {
            if (std::strcmp(_loadedTextures[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(_loadedTextures[j]);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), _directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            _loadedTextures.push_back(texture);
        }
    }

    return textures;
}