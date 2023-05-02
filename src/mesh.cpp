#include "mesh.h"

#include <vector>

GLint Mesh::VBO_ATTRIB_INDEX = 0;
GLint Mesh::TBO_ATTRIB_INDEX = 1;

Mesh::Mesh(Vertex* vertices, GLsizei num_vertices, GLuint* indices, GLsizei num_indices) : num_vertices(num_vertices),
	num_indices(num_indices) {
	// std::vector<glm::vec3> positions;
	// std::vector<glm::vec2> tex_coords;
	//
	// positions.reserve(num_vertices);
	// tex_coords.reserve(num_vertices);
	//
	// for (unsigned int i = 0; i < num_vertices / sizeof(Vertex); i++) {
	// 	positions.push_back(vertices[i].pos);
	// 	tex_coords.push_back(vertices[i].tex_coords);
	// }

	// float positions[] = {
	// 			-2.5f,  1.0f, 0.0f, // TL
	// 			-2.5f, -1.0f, 0.0f, // BL
	// 			1.0f, -1.0f, 0.0f,  // BR
	// 			1.0f,  1.0f, 0.0f,  // TR
	// };
	//
	// float tex_coords[] = {
	// 			0.0f, 1.0f, // TL
	// 			0.0f, 0.0f, // BL
	// 			1.0f, 0.0f, // BR
	// 			1.0f, 1.0f, // TR
	// };

	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	// glGenBuffers(1, &TBO);

	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	// glBindBuffer(GL_ARRAY_BUFFER, TBO);
	// glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(tex_coords[0]), &tex_coords, GL_STATIC_DRAW);

	glGenBuffers(1, &IBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(GLuint), indices, GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(VBO_ATTRIB_INDEX);
	glVertexAttribPointer(VBO_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), nullptr);

	// Vertex texture coordinates
	glEnableVertexAttribArray(TBO_ATTRIB_INDEX);
	glVertexAttribPointer(TBO_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
	                      (GLvoid*)offsetof(Vertex, tex_coords));


	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind IBO
	glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
	glBindVertexArray(0); // Unbind VAO

	num_vertices /= sizeof(Vertex);
}

void Mesh::bind() {
	glBindVertexArray(VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
}

void Mesh::unbind() {
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBindVertexArray(0);
}

void Mesh::draw_arrays() {
	bind();
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	unbind();
}

void Mesh::draw_elements() {
	bind();
	glDrawElements(GL_TRIANGLE_STRIP, num_indices, GL_UNSIGNED_INT, nullptr);
	unbind();
}
