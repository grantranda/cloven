#include "gui.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

Gui::Gui(GLFWwindow* glfw_window, std::vector<std::unique_ptr<GuiWindow>> windows) : windows(std::move(windows)) {
	// this->windows = std::move(windows);

	const char* glsl_version = "#version 460";

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	// Set ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

Gui::~Gui() {
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
}

void Gui::render() const {
	// Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create ImGui windows
	// ImGui::ShowDemoWindow();
	for (const auto& window : windows) {
		window->render();
	}

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
