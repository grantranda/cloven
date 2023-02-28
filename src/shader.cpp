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
		std::cout << "Error linking shader: " << info_log << "\n";
	}

	glValidateProgram(program_id);
	glGetProgramiv(program_id, GL_VALIDATE_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(program_id, 512, nullptr, info_log);
		std::cout << "Error validating shader: " << info_log << "\n";
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
		std::cout << "Error compiling shader: " << info_log << "\n";
		std::cout << "Shader path: " << shader_path << "\n";
	}

	glAttachShader(program_id, shader);
}

void Shader::set_uniform_1f(const std::string& name, const float x) const {
	glUniform1f(glGetUniformLocation(program_id, name.c_str()), x);
}

void Shader::set_uniform_vec4(const std::string& name, const glm::vec4 vec) const {
	glUniform4f(glGetUniformLocation(program_id, name.c_str()), vec.x, vec.y, vec.z, vec.w);
}
