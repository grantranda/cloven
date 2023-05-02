#pragma once
#include <string>
#include <GLFW/glfw3.h>

class Window {
public:
	GLFWwindow* glfw_window;

	Window(const std::string& title, const int width, const int height);
	~Window();

	bool should_close() const;
	void update();
	void render();

private:
	static void glfw_error_callback(int error, const char* description);
};
