#pragma once

#include <string>

#include <GLFW/glfw3.h>

class Window {
public:
	static constexpr float default_window_scale = 0.75f;

	GLFWwindow* glfw_window;

	Window(const std::string& title, int width = 0, int height = 0);
	~Window();

	[[nodiscard]] bool should_close() const;
	void update() const;
	void update_viewport(int x_offset = 0) const;
	void toggle_fullscreen();

private:
	bool is_fullscreen = false;
	int last_windowed_width = 1280;
    int last_windowed_height = 720;
    int last_windowed_pos_x = 0;
    int last_windowed_pos_y = 0;

	static void glfw_error_callback(int error, const char* description);
};
