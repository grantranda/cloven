#pragma once

#include <string>

#include "mesh.h"
#include "texture.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

class Model {
public:
	Model(const Mesh& mesh, const std::string& texture_path);
	Model(const Mesh& mesh, const std::string& texture_path, const glm::vec3& position, const glm::vec3& rotation,
	      const glm::vec3& scale);

	glm::mat4 model_matrix() const;
	void draw_arrays();
	void draw_elements();

private:
	Mesh mesh;
	Texture texture;
	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 scale;
};
