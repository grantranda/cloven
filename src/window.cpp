#include <imgui.h>

#include "window.h"
#include "app_settings.h"

void Window::glfw_error_callback(int error, const char* description) {
	fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

Window::Window(const std::string& title, int width, int height) {
	glfwSetErrorCallback(glfw_error_callback);
	if (!glfwInit()) {
		throw std::exception("Error initializing GLFW.");
	}

	// Set OpenGL version
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

	if (width == 0 || height == 0) {
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		width = static_cast<int>(static_cast<float>(mode->width) * default_window_scale);
		height = static_cast<int>(static_cast<float>(mode->height) * default_window_scale);
	}

	// Create window
	glfw_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!glfw_window) {
		glfwTerminate();
		throw std::exception("Error creating GLFW window.");
	}
	glfwMakeContextCurrent(glfw_window);
	glfwSwapInterval(1); // Enable VSync

	// Center window
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	const int window_pos_x = (mode->width - width) / 2;
    const int window_pos_y = (mode->height - height) / 2;
    glfwSetWindowPos(glfw_window, window_pos_x, window_pos_y);

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

void Window::toggle_fullscreen() {
	if (!glfw_window) {
		return;
	}

	if (is_fullscreen) {
		glfwSetWindowMonitor(glfw_window, nullptr, last_windowed_pos_x, last_windowed_pos_y, last_windowed_width, last_windowed_height, GLFW_DONT_CARE);
		is_fullscreen = false;
	} else {
		int width, height;
        glfwGetWindowSize(glfw_window, &width, &height);
        int pos_x, pos_y;
        glfwGetWindowPos(glfw_window, &pos_x, &pos_y);

        last_windowed_width = width;
        last_windowed_height = height;
        last_windowed_pos_x = pos_x;
        last_windowed_pos_y = pos_y;

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		const GLFWvidmode* mode = glfwGetVideoMode(monitor);
		glfwSetWindowMonitor(glfw_window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
		is_fullscreen = true;
	}
}
