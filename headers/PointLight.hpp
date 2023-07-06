#pragma once

#include <glm/vec3.hpp>

struct PointLight
{
	glm::vec3 position{};
	glm::vec3 color{};

	float constant { 1.0f };
	float linear { 0.14f };
	float quadratic { 0.07f };
};