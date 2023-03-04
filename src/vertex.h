#pragma once
#include <glm/glm.hpp>

struct Vertex {
	Vertex() = default;
	Vertex(const float x, const float y, const float z): pos(x, y, z) {}
	explicit Vertex(const glm::vec3 pos): pos(pos) {}
	Vertex(const glm::vec3 pos, const glm::vec2 tex_coords): pos(pos), tex_coords(tex_coords) {}

	glm::vec3 pos = glm::vec3(0.0f);
	glm::vec2 tex_coords = glm::vec2(0.0f);
};
