#include "mesh.h"

GLint Mesh::VBO_ATTRIB_INDEX = 0;
GLint Mesh::TBO_ATTRIB_INDEX = 1;

Mesh::Mesh(Vertex* vertices, GLsizei num_vertices, GLuint* indices, GLsizei num_indices) : num_vertices(num_vertices) {
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &TBO);
	glGenBuffers(1, &IBO);

	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, num_vertices * sizeof(Vertex), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(GLuint), indices, GL_STATIC_DRAW);

	// Vertex positions
	glEnableVertexAttribArray(VBO_ATTRIB_INDEX);
	glVertexAttribPointer(VBO_ATTRIB_INDEX, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Vertex texture coordinates
	glEnableVertexAttribArray(TBO_ATTRIB_INDEX);
	glVertexAttribPointer(TBO_ATTRIB_INDEX, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

	// glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0); // Unbind IBO
	// glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO
	glBindVertexArray(0); // Unbind VAO

	num_vertices /= sizeof(Vertex);
}

void Mesh::bind() {
	glBindVertexArray(VAO);
}

void Mesh::unbind() {
	glBindVertexArray(0);
}

void Mesh::draw_arrays() {
	bind();
	glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	unbind();
}

void Mesh::draw_elements() {
	bind();
	glDrawElements(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, nullptr);
	unbind();
}
