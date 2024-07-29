#pragma once

#include <string>

#include <GLFW/glfw3.h>

class Window {
public:
	GLFWwindow* glfw_window;

	Window(const std::string& title, const int width, const int height);
	~Window();

	[[nodiscard]] bool should_close() const;
	void update() const;
	void update_viewport(int x_offset = 0) const;

private:
	static void glfw_error_callback(int error, const char* description);
};
