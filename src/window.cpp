#include "window.h"

#include <imgui.h>

void Window::glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Window::Window(const std::string& title, const int width, const int height) {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		throw std::exception("Error initializing GLFW.");
	}

	// Set OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	// Create window
	glfw_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!glfw_window) {
		glfwTerminate();
		throw std::exception("Error creating GLFW window.");
	}
	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1); // Enable VSync

	glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

Window::~Window() {
	glfwDestroyWindow(glfw_window);
	glfwTerminate();
}

bool Window::should_close() const {
	return glfwWindowShouldClose(glfw_window);
}

void Window::update() const {
	glfwSwapBuffers(glfw_window);
	glfwPollEvents();
}

void Window::update_viewport(const int x_offset) const {
	int display_w, display_h;
	glfwGetFramebufferSize(glfw_window, &display_w, &display_h);
	const int available_width = display_w - x_offset;
	const GLfloat screen_ratio = static_cast<GLfloat>(display_w) / static_cast<GLfloat>(display_h);
	const int viewport_height = static_cast<int>(static_cast<float>(available_width) / screen_ratio);
	glViewport(x_offset, (display_h - viewport_height) / 2, available_width, viewport_height);
}
