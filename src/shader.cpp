#include "shader.h"

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path) {
	program_id = glCreateProgram();

	attach_shader(vertex_path, GL_VERTEX_SHADER);
	attach_shader(fragment_path, GL_FRAGMENT_SHADER);

	glLinkProgram(program_id);

	int success;
	char info_log[512];

	glGetProgramiv(program_id, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program_id, 512, nullptr, info_log);
		fprintf(stderr, "Error linking shader: %s\n", info_log);
	}

	glValidateProgram(program_id);
	glGetProgramiv(program_id, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program_id, 512, nullptr, info_log);
		fprintf(stderr, "Error validating shader: %s\n", info_log);
	}
}

Shader::~Shader() {
	if (program_id != 0) {
		glDeleteProgram(program_id);
		program_id = 0;
	}
}

void Shader::bind() {
	glUseProgram(program_id);
}

std::string Shader::read_file(const std::string& path) {
	std::ifstream file(path);
	std::string content(
		(std::istreambuf_iterator<char>(file)),
		(std::istreambuf_iterator<char>())
	);
	return content;
}

void Shader::attach_shader(const std::string& shader_path, const GLenum shader_type) {
	const std::string shader_string = read_file(shader_path);

	const GLchar* source[1];
	source[0] = shader_string.c_str();

	GLint source_length[1];
	source_length[0] = strlen(shader_string.c_str());

	const unsigned int shader = glCreateShader(shader_type);
	glShaderSource(shader, 1, source, source_length);
	glCompileShader(shader);

	int success;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success) {
		char info_log[512];
		glGetShaderInfoLog(shader, 512, nullptr, info_log);
		fprintf(stderr, "Error compiling shader: %s\n", info_log);
		fprintf(stderr, "Shader path: %s\n", shader_path.c_str());
	}

	glAttachShader(program_id, shader);
}

void Shader::set_uniform_1i(const std::string& name, const int x) const {
	glUniform1i(glGetUniformLocation(program_id, name.c_str()), x);
}

void Shader::set_uniform_1f(const std::string& name, const float x) const {
	glUniform1f(glGetUniformLocation(program_id, name.c_str()), x);
}

void Shader::set_uniform_2f(const std::string& name, const float x, const float y) const {
	glUniform2f(glGetUniformLocation(program_id, name.c_str()), x, y);
}

void Shader::set_uniform_1d(const std::string& name, const double x) const {
	glUniform1d(glGetUniformLocation(program_id, name.c_str()), x);
}

void Shader::set_uniform_2d(const std::string& name, const double x, const double y) const {
	glUniform2d(glGetUniformLocation(program_id, name.c_str()), x, y);
}

void Shader::set_uniform_vec3(const std::string& name, const glm::vec3 vec) const {
	glUniform3f(glGetUniformLocation(program_id, name.c_str()), vec.x, vec.y, vec.z);
}

void Shader::set_uniform_vec4(const std::string& name, const glm::vec4 vec) const {
	glUniform4f(glGetUniformLocation(program_id, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}

void Shader::set_uniform_mat4(const std::string& name, glm::mat4 mat) const {
	glUniformMatrix4fv(glGetUniformLocation(program_id, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}
