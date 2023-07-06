#pragma once

#include <string>
#include <map>

class TextureManager
{
private:

public:
	TextureManager() = default;

	unsigned int load(const std::string& fileName);
	void activate(unsigned int level, unsigned int id) const;
};