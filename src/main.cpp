#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "gui.h"
#include "shader.h"
#include "window.h"
#include "windows/test_window.h"

int main() {
	Window window("Cloven", 1920, 1080);

	std::vector<std::unique_ptr<GuiWindow>> windows;
	windows.push_back(std::make_unique<TestWindow>());
	
	const Gui gui(window.glfw_window, std::move(windows));

	while (!window.should_close()) {
		window.update();
		window.render();
		gui.render();
	}

	return 0;
}
