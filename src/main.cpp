#include <iostream>
#include <vector>
#include <algorithm>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "window.h"
#include "camera.h"
#include "shader.h"
#include "gradient_editor.h"
#include "app_settings.h"

// Global variables
AppSettings settings;
Window* window;
Camera camera;
GradientEditor gradient_editor;
glm::vec2 resolution = glm::vec2(default_width, default_height);
GLfloat aspect_ratio = resolution.x / resolution.y;
std::vector<unsigned char> gradient_data(768); // 256 * 1 * 3 = 768

// Function declarations
void key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* glfw_window, double xpos, double ypos);
void resize_callback(GLFWwindow* glfw_window, const int w, const int h);
void init_gui(GLFWwindow* glfw_window);
bool slider_int(const char* label, int* v, const int v_min, const int v_max, const int v_default, const char* format = "%d", const ImGuiSliderFlags flags = 0);
bool slider_float(const char* label, float* v, const float v_min, const float v_max, const float v_default, const char* format = "%.3f", const ImGuiSliderFlags flags = 0);
bool drag_float3(const char* label, float v[3], const float v_min, const float v_max, const float v_default[3], const char* format = "%.3f", const ImGuiSliderFlags flags = 0);
void gradient_preview(int width, int height);
void show_gradient_editor();
void show_main_window();
void render_gui();

int main() {
	// Initialize window
	try {
		window = new Window("Cloven", default_width, default_height);
	} catch (std::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}

	// Window callbacks
	glfwSetKeyCallback(window->glfw_window, key_callback);
	glfwSetCursorPosCallback(window->glfw_window, cursor_position_callback);
	glfwSetFramebufferSizeCallback(window->glfw_window, resize_callback);
	resize_callback(window->glfw_window, default_width, default_height);

	// Initialize GLEW
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_error));
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.10f, 0.10f, 0.10f, 1.0f);

	init_gui(window->glfw_window);

	// Create and bind shader
	Shader shader("shaders/shader.vert", "shaders/shader.frag");

	shader.bind();

	// Set up VAO and VBO
	constexpr float quad_vertices[] = {
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		 1.0f, -1.0f,
	
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f
	};
	unsigned int quad_vao, quad_vbo;
	glGenVertexArrays(1, &quad_vao);
	glGenBuffers(1, &quad_vbo);
	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	// Generate and bind gradient texture
	GLuint texture_id = 0;
	glGenTextures(1, &texture_id);
	glBindTexture(GL_TEXTURE_1D, texture_id);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	constexpr GLuint texture_unit = 0;
	glActiveTexture(GL_TEXTURE0 + texture_unit);
	glBindTexture(GL_TEXTURE_1D, texture_id);

	camera = Camera();
	int nb_frames = 0;
	double current_time = glfwGetTime();
	double last_update_time = current_time;
	double last_frame_time = current_time;

	// Main update_viewport loop
	while (!window->should_close()) {
		// Update FPS
		nb_frames++;
		current_time = glfwGetTime();
		settings.update_delta_time = current_time - last_update_time;
		settings.frame_delta_time = current_time - last_frame_time;
		last_frame_time = current_time;
		if (settings.update_delta_time >= 1.0) {
			settings.fps = nb_frames;
			nb_frames = 0;
			last_update_time += 1.0;
		}

		// Handle camera input
		if (glfwGetInputMode(window->glfw_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			camera.handle_keyboard_input(window->glfw_window, static_cast<float>(settings.frame_delta_time));
		}

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Set uniforms
		shader.bind();
		const glm::mat4 projection_matrix = glm::perspective(glm::radians(camera.zoom), aspect_ratio, 0.1f, 100.0f);
		const glm::mat4 inverse_view_matrix = glm::inverse(camera.view_matrix());
		const glm::mat4 inverse_projection_matrix = glm::inverse(projection_matrix);

		shader.set_uniform_mat4("u_inverse_view_matrix", inverse_view_matrix);
		shader.set_uniform_mat4("u_inverse_projection_matrix", inverse_projection_matrix);
		shader.set_uniform_2f("u_resolution", default_width, default_height);
		shader.set_uniform_vec3("u_camera_pos", camera.position);
		shader.set_uniform_1i("u_enable_normal_visualization", settings.enable_normal_visualization);

		shader.set_uniform_1i("u_max_iterations", settings.max_iterations);
		shader.set_uniform_1i("u_escape_radius", settings.escape_radius);
		shader.set_uniform_1i("u_step_limit", settings.step_limit);
		shader.set_uniform_1f("u_max_distance", settings.max_distance);
		shader.set_uniform_1f("u_power", settings.power);
		shader.set_uniform_1f("u_epsilon", settings.epsilon);
		shader.set_uniform_1f("u_ray_hit_threshold", settings.ray_hit_threshold);

		shader.set_uniform_1i("u_coloring_method", settings.coloring_method);
		shader.set_uniform_1i("u_background_type", settings.background_type);
		shader.set_uniform_vec3("u_background_color", glm::vec3(
			settings.background_color[0],
			settings.background_color[1],
			settings.background_color[2]
		));

		shader.set_uniform_vec3("u_light_pos", settings.light_pos);
		shader.set_uniform_1f("u_light_power", settings.light_power);
		shader.set_uniform_1f("u_noise_scale", settings.noise_scale);
		shader.set_uniform_1f("u_noise_amplitude", settings.noise_amplitude);
		shader.set_uniform_1f("u_ambient_strength", settings.ambient_strength);
		shader.set_uniform_1f("u_diffuse_strength", settings.diffuse_strength);
		shader.set_uniform_1f("u_specular_strength", settings.specular_strength);
		shader.set_uniform_1f("u_specular_shininess", settings.specular_shininess);
		shader.set_uniform_1f("u_shadow_softness", settings.shadow_softness);
		shader.set_uniform_1f("u_shadow_min_distance", settings.shadow_min_distance);
		shader.set_uniform_1f("u_shadow_min_step_size", settings.shadow_min_step_size);
		shader.set_uniform_1f("u_shadow_max_step_size", settings.shadow_max_step_size);
		shader.set_uniform_1i("u_shadow_max_iterations", settings.shadow_max_iterations);
		shader.set_uniform_1f("u_bloom_intensity_factor", settings.bloom_intensity_factor);
		shader.set_uniform_vec3("u_bloom_color", glm::vec3(
			settings.bloom_color[0],
			settings.bloom_color[1],
			settings.bloom_color[2]
		));
		shader.set_uniform_1f("u_light_radius", settings.light_radius);
		shader.set_uniform_vec3("u_light_color", glm::vec3(
			settings.light_color[0],
			settings.light_color[1],
			settings.light_color[2]
		));
		shader.set_uniform_1i("u_show_light", settings.show_light);
		shader.set_uniform_1i("u_apply_noise", settings.apply_noise);
		shader.set_uniform_1i("u_apply_blinn_phong", settings.apply_blinn_phong);
		shader.set_uniform_1i("u_apply_soft_shadow", settings.apply_soft_shadow);
		shader.set_uniform_1i("u_apply_bloom", settings.apply_bloom);
		shader.set_uniform_1i("u_apply_ambient_occlusion", settings.apply_ambient_occlusion);

		// Update gradient texture
		glActiveTexture(GL_TEXTURE0 + texture_unit);
		glBindTexture(GL_TEXTURE_1D, texture_id);
		shader.set_uniform_1i("u_gradient_texture", static_cast<int>(texture_unit));
		for (size_t i = 0; i < 256; ++i) {
		    const float position = static_cast<float>(i) / 255.0f;
			const ImVec4 color = gradient_editor.interpolate(position);
			gradient_data[i * 3] = static_cast<unsigned char>(color.x * 255.0f);
		    gradient_data[i * 3 + 1] = static_cast<unsigned char>(color.y * 255.0f);
		    gradient_data[i * 3 + 2] = static_cast<unsigned char>(color.z * 255.0f);
		}
		glTexImage1D(GL_TEXTURE_1D, 0, GL_RGB, 256, 0, GL_RGB, GL_UNSIGNED_BYTE, gradient_data.data());

		// Draw the scene
		glBindVertexArray(quad_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);

		// Render the GUI if visible
		if (settings.show_gui) {
			render_gui();
		}

		// Update window
		window->update();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();
	glDeleteVertexArrays(1, &quad_vao);
	glDeleteBuffers(1, &quad_vbo);
	delete window;

	return 0;
}

void key_callback(GLFWwindow* glfw_window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(glfw_window, true);
	} else if (key == GLFW_KEY_LEFT_ALT && action == GLFW_PRESS) {
		if (glfwGetInputMode(glfw_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(glfw_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	} else if (key == GLFW_KEY_F && action == GLFW_PRESS) {
		settings.show_gui = !settings.show_gui;
		window->update_viewport(settings.show_gui ? gui_width : 0);
	}
}

void cursor_position_callback(GLFWwindow* glfw_window, double xpos, double ypos) {
	static bool first_mouse = true;
	static double last_x = xpos;
	static double last_y = ypos;

	if (first_mouse) {
		last_x = xpos;
		last_y = ypos;
		first_mouse = false;
	}

	const double delta_x = xpos - last_x;
	const double delta_y = last_y - ypos;

	last_x = xpos;
	last_y = ypos;

	if (glfwGetInputMode(glfw_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		camera.handle_mouse_movement(static_cast<float>(delta_x), static_cast<float>(delta_y));
	}
}

void resize_callback(GLFWwindow* glfw_window, const int w, const int h) {
	window->update_viewport(settings.show_gui ? gui_width : 0);
	resolution.x = static_cast<GLfloat>(w);
	resolution.y = static_cast<GLfloat>(h);
	aspect_ratio = resolution.x / resolution.y;
}

void init_gui(GLFWwindow* glfw_window) {
	auto glsl_version = "#version 460";

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	io.ConfigWindowsMoveFromTitleBarOnly = true;

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);

	gradient_editor = GradientEditor();
}

bool slider_int(const char* label, int* v, const int v_min, const int v_max, const int v_default, const char* format, const ImGuiSliderFlags flags) {
	const bool result = ImGui::SliderInt(label, v, v_min, v_max, format, flags);

	if (ImGui::BeginPopupContextItem(label)) {
		char buffer[64];
		std::string reset_str = "Reset to ";
		reset_str += format;
		sprintf_s(buffer, 64, reset_str.c_str(), v_default);
		if (ImGui::MenuItem(buffer)) *v = v_default;
		ImGui::MenuItem("Close");
		ImGui::EndPopup();
	}
	return result;
}

bool slider_float(const char* label, float* v, const float v_min, const float v_max, const float v_default, const char* format, const ImGuiSliderFlags flags) {
	const bool result = ImGui::SliderFloat(label, v, v_min, v_max, format, flags);

	if (ImGui::BeginPopupContextItem(label)) {
		char buffer[64];
		std::string reset_str = "Reset to ";
		reset_str += format;
		sprintf_s(buffer, 64, reset_str.c_str(), v_default);
		if (ImGui::MenuItem(buffer)) *v = v_default;
		ImGui::MenuItem("Close");
		ImGui::EndPopup();
	}
	return result;
}

bool drag_float3(const char* label, float v[3], const float v_min, const float v_max, const float v_default[3], const char* format, const ImGuiSliderFlags flags) {
	const bool result = ImGui::DragFloat3(label, v, 1, v_min, v_max, format, flags);

	if (ImGui::BeginPopupContextItem(label)) {
		char buffer[64];
		const std::string reset_str = "Reset to (%.1f, %.1f, %.1f)";
		sprintf_s(buffer, 64, reset_str.c_str(), v_default[0], v_default[1], v_default[2]);
		if (ImGui::MenuItem(buffer)) {
			v[0] = v_default[0];
			v[1] = v_default[1];
			v[2] = v_default[2];
		}
		ImGui::MenuItem("Close");
		ImGui::EndPopup();
	}
	return result;
}

void gradient_preview(const int width, const int height) {
	const ImVec2 canvas_size(width, height);
    const ImVec2 gradient_pos = ImGui::GetCursorScreenPos();

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    for (int i = 0; i < width; ++i) {
        const ImVec2 p0(gradient_pos.x + i, gradient_pos.y);
        const ImVec2 p1(gradient_pos.x + i, gradient_pos.y + canvas_size.y);
		const auto position = static_cast<float>(i) / width;
		const auto color = gradient_editor.interpolate(position);
		const ImU32 col = IM_COL32(
			static_cast<int>(color.x * 255),
            static_cast<int>(color.y * 255),
            static_cast<int>(color.z * 255),
            static_cast<int>(color.w * 255)
		);

        draw_list->AddLine(p0, p1, col);
    }

	ImGui::Dummy(canvas_size);
}

void show_gradient_editor() {
    constexpr ImVec2 window_size(440, 390);
	const ImVec2 main_window_size = ImGui::GetIO().DisplaySize;
	const auto window_pos = ImVec2(
        480.0f,
        (main_window_size.y - window_size.y) / 2.0f
    );

    ImGui::SetNextWindowPos(window_pos, ImGuiCond_Once);
    ImGui::SetNextWindowSize(window_size, ImGuiCond_Once);
	ImGui::Begin("Gradient Editor", &settings.show_gradient_editor, ImGuiWindowFlags_NoResize);
	
	gradient_editor.show();

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5);

	if (ImGui::Button("Random")) {
        gradient_editor.random_gradient();
    }
	ImGui::SameLine();
	if (ImGui::Button("Reset")) {
        gradient_editor.set_default_gradient();
    }
	ImGui::SameLine();
	if (ImGui::Button("Close")) {
        settings.show_gradient_editor = false;
    }

	ImGui::End();
}

void show_main_window() {
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(gui_width, resolution.y));

	ImGui::Begin("Main Window", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);

	if (ImGui::CollapsingHeader("Fractal", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::SeparatorText("General##Fractal");
		slider_int("Max Iterations##Fractal", &settings.max_iterations, 1, 100, default_max_iterations, "%d");
		slider_int("Escape Radius##Fractal", &settings.escape_radius, 1, 1000, default_escape_radius, "%d");
		slider_float("Power##Fractal", &settings.power, -64.0f, 64.0f, default_power, "%.3f");
		ImGui::SeparatorText("Ray Marching##Fractal");
		slider_float("Epsilon##Fractal", &settings.epsilon, 0.0000001f, 0.01f, default_epsilon, "%.7f");
		slider_float("Max Distance##Fractal", &settings.max_distance, 0.0f, 100.0f, default_max_distance, "%.1f");
		slider_float("Ray Hit Threshold##Fractal", &settings.ray_hit_threshold, 0.0f, 1.0f, default_ray_hit_threshold, "%.5f");
		slider_int("Step Limit##Fractal", &settings.step_limit, 1, 1000, default_step_limit, "%d");
		if (ImGui::Button("Reset Fractal")) {
			settings.max_iterations = default_max_iterations;
			settings.escape_radius = default_escape_radius;
			settings.step_limit = default_step_limit;
			settings.power = default_power;
			settings.epsilon = default_epsilon;
			settings.max_distance = default_max_distance;
			settings.ray_hit_threshold = default_ray_hit_threshold;
		}
	}

	if (ImGui::CollapsingHeader("Coloring", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Combo("Method##Coloring", &settings.coloring_method, "Distance-Based\0Orbit Trap\0\0");
		ImGui::SeparatorText("Gradient##Coloring");
		gradient_preview(256, 17);
		ImGui::SameLine();
		if (ImGui::Button("Edit Gradient##Coloring")) {
			settings.show_gradient_editor = true;
		}
		ImGui::SeparatorText("Background##Coloring");
		ImGui::Combo("Type##Background", &settings.background_type, "Solid\0Dynamic\0\0");
		if (settings.background_type == 1) ImGui::BeginDisabled();
		ImGui::ColorEdit3("Color##Background", settings.background_color, ImGuiColorEditFlags_Float);
		if (settings.background_type == 1) ImGui::EndDisabled();
		if (ImGui::Button("Reset Coloring")) {
			settings.coloring_method = 0;
			settings.background_type = 0;
			settings.background_color[0] = default_background_color[0];
			settings.background_color[1] = default_background_color[1];
			settings.background_color[2] = default_background_color[2];
		}
	}

	if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Show Light Source##Lighting", &settings.show_light);
		drag_float3("Position##Lighting", reinterpret_cast<float*>(&settings.light_pos), 0, 0, default_light_pos);
		slider_float("Power##Lighting", &settings.light_power, -100, 100, default_light_power, "%.7f");
		slider_float("Radius##Lighting", &settings.light_radius, 0, 1, default_light_radius, "%.7f");
		ImGui::ColorEdit3("Color##Lighting", settings.light_color, ImGuiColorEditFlags_Float);
		if (ImGui::Button("Reset Lighting")) {
			settings.show_light = true;
			settings.light_pos[0] = default_light_pos[0];
			settings.light_pos[1] = default_light_pos[1];
			settings.light_pos[2] = default_light_pos[2];
			settings.light_power = default_light_power;
			settings.light_radius = default_light_radius;
			settings.light_color[0] = default_light_color[0];
			settings.light_color[1] = default_light_color[1];
			settings.light_color[2] = default_light_color[2];
		}
	}

	if (ImGui::CollapsingHeader("Noise", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Apply Noise##Noise", &settings.apply_noise);
		slider_float("Scale##Noise", &settings.noise_scale, 0, 100, default_noise_scale, "%.7f");
		slider_float("Amplitude##Noise", &settings.noise_amplitude, 0, 10, default_noise_amplitude, "%.7f");
		if (ImGui::Button("Reset Noise")) {
			settings.apply_noise = true;
			settings.noise_scale = default_noise_scale;
			settings.noise_amplitude = default_noise_amplitude;
		}
	}

	if (ImGui::CollapsingHeader("Shading", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Apply Blinn-Phong##Shading", &settings.apply_blinn_phong);
		slider_float("Ambient Strength##Shading", &settings.ambient_strength, -10, 10, default_ambient_strength, "%.7f");
		slider_float("Diffuse Strength##Shading", &settings.diffuse_strength, -10, 10, default_diffuse_strength, "%.7f");
		slider_float("Specular Strength##Shading", &settings.specular_strength, -10, 10, default_specular_strength, "%.7f");
		slider_float("Specular Shininess##Shading", &settings.specular_shininess, -100, 100, default_specular_shininess, "%.7f");
		if (ImGui::Button("Reset Shading")) {
			settings.apply_blinn_phong = true;
			settings.ambient_strength = default_ambient_strength;
			settings.diffuse_strength = default_diffuse_strength;
			settings.specular_strength = default_specular_strength;
			settings.specular_shininess = default_specular_shininess;
		}
	}

	if (ImGui::CollapsingHeader("Shadows", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Apply Soft Shadows##Shadows", &settings.apply_soft_shadow);
		slider_float("Softness##Shadows", &settings.shadow_softness, 0, 100, default_shadow_softness, "%.7f");
		slider_float("Min Distance##Shadows", &settings.shadow_min_distance, 0, 1, default_shadow_min_distance, "%.7f");
		slider_float("Min Step Size##Shadows", &settings.shadow_min_step_size, 0, 1, default_shadow_min_step_size, "%.7f");
		slider_float("Max Step Size##Shadows", &settings.shadow_max_step_size, 0, 1, default_shadow_max_step_size, "%.7f");
		slider_int("Max Iterations##Shadows", &settings.shadow_max_iterations, 0, 1000, default_shadow_max_iterations, "%d");
		if (ImGui::Button("Reset Shadows")) {
			settings.apply_soft_shadow = true;
			settings.shadow_softness = default_shadow_softness;
			settings.shadow_min_distance = default_shadow_min_distance;
			settings.shadow_min_step_size = default_shadow_min_step_size;
			settings.shadow_max_step_size = default_shadow_max_step_size;
			settings.shadow_max_iterations = default_shadow_max_iterations;
		}
	}

	if (ImGui::CollapsingHeader("Bloom", ImGuiTreeNodeFlags_DefaultOpen)) {
		ImGui::Checkbox("Apply Bloom##Bloom", &settings.apply_bloom);
		slider_float("Intensity Factor##Bloom", &settings.bloom_intensity_factor, 0, 10, default_bloom_intensity_factor, "%.7f");
		ImGui::ColorEdit3("Color##Bloom", settings.bloom_color, ImGuiColorEditFlags_Float);
		if (ImGui::Button("Reset Bloom")) {
			settings.apply_bloom = true;
			settings.bloom_intensity_factor = default_bloom_intensity_factor;
			settings.bloom_color[0] = default_bloom_color[0];
			settings.bloom_color[1] = default_bloom_color[1];
			settings.bloom_color[2] = default_bloom_color[2];
		}
	}

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
		drag_float3("Position##Camera", reinterpret_cast<float*>(&camera.position), 0, 0, default_camera_pos);
		slider_float("Field of View##Camera", &camera.zoom, 1.0f, 120.0f, Camera::default_zoom, "%.1f", ImGuiSliderFlags_AlwaysClamp);
		slider_float("Speed##Camera", &camera.speed, 0.0f, 1.0f, Camera::default_speed, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		slider_float("Sensitivity##Camera", &camera.sensitivity, 0.01f, 1.0f, Camera::default_sensitivity, "%.2f", ImGuiSliderFlags_AlwaysClamp);
		if (ImGui::Button("Reset Camera")) {
			camera.reset();
		}
	}

	if (ImGui::CollapsingHeader("Debug")) {
		ImGui::Checkbox("Enable Normal Visualization##Misc", &settings.enable_normal_visualization);
		ImGui::Text("FPS: %d", settings.fps);
	}

	ImGui::End();
}

void render_gui() {
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, 3.0f);

	show_main_window();

	if (settings.show_gradient_editor) {
		show_gradient_editor();
	}

	ImGui::PopStyleVar(3);

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
