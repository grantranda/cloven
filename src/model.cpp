#include "model.h"

Model::Model(const Mesh& mesh, const std::string& texture_path) : Model(mesh, texture_path, glm::vec3(0.0),
                                                                        glm::vec3(0.0), glm::vec3(1.0)) { }

Model::Model(const Mesh& mesh, const std::string& texture_path, const glm::vec3& position, const glm::vec3& rotation,
             const glm::vec3& scale) : mesh(mesh), position(position), rotation(rotation), scale(scale) {
	texture = Texture(GL_TEXTURE_1D, texture_path);
	texture.load();
}

glm::mat4 Model::model_matrix() const {
	glm::mat4 model(1.0f);

	model = translate(model, position);
	model = rotate(model, glm::radians(rotation.x), glm::vec3(1.0f, 0.0f, 0.0f));
	model = rotate(model, glm::radians(rotation.y), glm::vec3(0.0f, 1.0f, 0.0f));
	model = rotate(model, glm::radians(rotation.z), glm::vec3(0.0f, 0.0f, 1.0f));
	model = glm::scale(model, scale);

	return model;
}

void Model::draw_arrays() {
	texture.bind();
	mesh.draw_arrays();
	texture.unbind();
}

void Model::draw_elements() {
	texture.bind();
	mesh.draw_elements();
	texture.unbind();
}
