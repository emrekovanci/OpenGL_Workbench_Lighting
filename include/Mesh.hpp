#pragma once

#include "Vertex.hpp"
#include "Texture.hpp"
#include "Shader.hpp"

#include <vector>

class Mesh
{
public:
    Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const std::vector<Texture>& textures);
    void render(const Shader& shader) const;

private:
    std::vector<Vertex> _vertices;
    std::vector<unsigned int> _indices;
    std::vector<Texture> _textures;

    unsigned int _vao;
    unsigned int _vbo;
    unsigned int _ebo;

private:
    void initialize();
};