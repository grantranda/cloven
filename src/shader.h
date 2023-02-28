#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <GL/gl.h>

class Shader {
public:
	Shader(const std::string& vertex_path, const std::string& fragment_path);
	~Shader();

	void bind();

	void set_uniform_1f(const std::string& name, float x) const;
	void set_uniform_vec4(const std::string& name, glm::vec4 vec) const;

private:
	GLuint program_id;
	std::string read_file(const std::string& path);
	void attach_shader(const std::string& shader_path, GLenum shader_type);
};
