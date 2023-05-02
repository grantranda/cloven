#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GL/gl.h>

class Shader {
public:
	Shader(const std::string& vertex_path, const std::string& fragment_path);
	~Shader();

	void bind();

	void set_uniform_1i(const std::string& name, int x) const;
	void set_uniform_1f(const std::string& name, float x) const;
	void set_uniform_2f(const std::string& name, float x, float y) const;
	void set_uniform_1d(const std::string& name, double x) const;
	void set_uniform_2d(const std::string& name, double x, double y) const;
	void set_uniform_vec3(const std::string& name, glm::vec3 vec) const;
	void set_uniform_vec4(const std::string& name, glm::vec4 vec) const;
	void set_uniform_mat4(const std::string& name, glm::mat4 mat) const;

private:
	GLuint program_id;
	std::string read_file(const std::string& path);
	void attach_shader(const std::string& shader_path, GLenum shader_type);
};
