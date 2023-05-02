#include "texture.h"

#include <GL/glew.h>

#define STB_IMAGE_IMPLEMENTATION
#include "../external/stb/stb_image.h"

Texture::Texture(const GLenum type, const std::string& path) : id(0), type(type), path(std::move(path)) { }

Texture::~Texture() {
	delete_texture();
}

void Texture::load() {
	stbi_set_flip_vertically_on_load(1);

	int width, height, num_channels;
	unsigned char* data = stbi_load(path.c_str(), &width, &height, &num_channels, 0);
	if (!data) {
		fprintf(stderr, "Error loading texture: %s\n", path.c_str());
		return;
	}

	glGenTextures(1, &id);
	glBindTexture(type, id);

	switch (type) {
	case GL_TEXTURE_1D:
		glTexImage1D(type, 0, GL_RGBA, width, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	case GL_TEXTURE_2D:
		glTexImage2D(type, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		break;
	default:
		fprintf(stderr, "Error - unsupported texture type: %u\n", type);
		break;
	}

	glTexParameteri(type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(type, GL_TEXTURE_BASE_LEVEL, 0);
	glTexParameteri(type, GL_TEXTURE_MAX_LEVEL, 0);

	stbi_image_free(data);
}

void Texture::bind() const {
	glBindTexture(type, id);
}

void Texture::unbind() const {
	glBindTexture(type, 0);
}

void Texture::delete_texture() const {
	glDeleteTextures(1, &id);
}
