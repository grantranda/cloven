#pragma once
#include <string>
#include <GL/glew.h>

class Texture {
public:
	Texture() = default;
	Texture(GLenum type, const std::string& path);
	~Texture();

	void load();
	void bind() const;
	void unbind() const;
	void delete_texture() const;

private:
	GLuint id;
	GLenum type;
	std::string path;
};
