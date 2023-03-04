#pragma once
#include <memory>
#include <vector>

#include "window.h"
#include "windows/gui_window.h"

class Gui {
public:
	Gui(GLFWwindow* glfw_window, std::vector<std::unique_ptr<GuiWindow>> windows);
	~Gui();

	void render() const;

private:
	std::vector<std::unique_ptr<GuiWindow>> windows;
};
