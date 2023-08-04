#pragma once

#include <string>
#include <map>

class TextureManager
{
public:
    void load(const std::string& fileName, const std::string& identifier);
    unsigned int get(const std::string& identifier);

    void activate(unsigned int level, unsigned int id) const;

private:
    std::map<std::string, unsigned int> _textures;
};