#include "window.h"

#include <imgui.h>

void Window::glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Window::Window(const std::string& title, int width, int height) {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		// return -1;
	}

	// Set OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	// Create window
	glfw_window = glfwCreateWindow(1920, 1080, "Cloven", nullptr, nullptr);
	if (!glfw_window) {
		glfwTerminate();
		// return -1;
	}
	glfwMakeContextCurrent(glfw_window);
	// glfwSwapInterval(1); // Enable VSync

	glfwSetInputMode(glfw_window, GLFW_STICKY_KEYS, GL_TRUE);
}

Window::~Window() {
	glfwDestroyWindow(glfw_window);
	glfwTerminate();
}

bool Window::should_close() const {
	return glfwWindowShouldClose(glfw_window);
}

void Window::update() {
	glfwSwapBuffers(glfw_window);
	glfwPollEvents();
}

void Window::render() {
	constexpr ImVec4 clear_color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);

	int display_w, display_h;
	glfwGetFramebufferSize(glfw_window, &display_w, &display_h);
	glViewport(0, 0, display_w, display_h);
	glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w,
		clear_color.w);
	glClear(GL_COLOR_BUFFER_BIT);
}
