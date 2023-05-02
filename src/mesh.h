#pragma once
#include <GL/glew.h>

#include "vertex.h"

class Mesh {
public:
	Mesh(Vertex* vertices, GLsizei num_vertices, GLuint* indices = nullptr, GLsizei num_indices = 0);
	~Mesh() = default;

	void bind();
	void unbind();
	void draw_arrays();
	void draw_elements();

private:
	static GLint VBO_ATTRIB_INDEX;
	static GLint TBO_ATTRIB_INDEX;

	GLuint VAO, VBO, TBO, IBO;
	GLsizei num_vertices;
	GLsizei num_indices;
};
