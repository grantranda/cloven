#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "camera.h"
#include "mesh.h"
#include "model.h"
#include "shader.h"
#include "vertex.h"
#include "window.h"

constexpr int default_width = 1920;
constexpr int default_height = 1080;

constexpr float default_power = 8.0f;
constexpr float default_epsilon = 0.0001f;
constexpr float default_max_distance = 50.0f;
constexpr float default_ray_hit_threshold = 0.00001f;

glm::vec2 screen_size = glm::vec2(default_width, default_height);
GLfloat screen_ratio = screen_size.x / screen_size.y;
Camera camera;

struct AppState {
	int max_iterations = 20;
	int escape_radius = 4;
	int step_limit = 1000;
	float power = default_power;
	float epsilon = default_epsilon;
	float max_distance = default_max_distance;
	float ray_hit_threshold = default_ray_hit_threshold;

	bool enable_normal_visualization = false;

	int fps = 0;
	// The time since the FPS was last updated
	double update_delta_time = 0.0;
	// The time since the last frame was rendered - used in calculating the camera's position
	double frame_delta_time = 0.0;
} app_state;

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
	} else if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
		if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		} else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}
	}
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos) {
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

	if (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
		camera.handle_mouse_movement(static_cast<float>(delta_x), static_cast<float>(delta_y));
	}
}

void scroll_callback(GLFWwindow* window, const double x_offset, const double y_offset) {
	camera.handle_mouse_scroll(static_cast<float>(y_offset));
}

void resize_callback(GLFWwindow* win, const int w, const int h) {
	glViewport(0, 0, w, h);
	screen_size.x = static_cast<GLfloat>(w);
	screen_size.y = static_cast<GLfloat>(h);
	screen_ratio = screen_size.x / screen_size.y;
}

void init_gui(GLFWwindow* glfw_window) {
	auto glsl_version = "#version 460";

	// Initialize ImGui
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	const ImGuiIO& io = ImGui::GetIO();
	(void)io;

	// Set ImGui style
	ImGui::StyleColorsDark();
	// ImGui::StyleColorsLight();

	ImGui_ImplGlfw_InitForOpenGL(glfw_window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

bool slider_float(const char* label, float* v, const float v_min, const float v_max, const float v_default, const char* format = "%.3f") {
	const bool result = ImGui::SliderFloat(label, v, v_min, v_max, format);

	if (ImGui::BeginPopupContextItem(label)) {
		char buffer[64];

		sprintf_s(buffer, 64, "Reset to %f", v_default);
		if (ImGui::MenuItem(buffer)) *v = v_default;
		ImGui::MenuItem("Close");
		ImGui::EndPopup();
	}
	return result;
}

void create_main_window() {
	ImGui::Begin("Main Window");

	ImGui::Text("FPS: %d", app_state.fps);
	ImGui::InputInt("Max Iterations", &app_state.max_iterations);
	ImGui::InputInt("Escape Radius", &app_state.escape_radius);
	slider_float("Power", &app_state.power, -64.0f, 64.0f, default_power, "%.3f");
	slider_float("Epsilon", &app_state.epsilon, 0.0000001f, 0.01f, default_epsilon, "%.7f");
	slider_float("Max Distance", &app_state.max_distance, 0.0f, 100.0f, default_max_distance, "%.1f");
	slider_float("Ray Hit Threshold", &app_state.ray_hit_threshold, 0.0f, 1.0f, default_ray_hit_threshold, "%.5f");
	ImGui::InputInt("Step Limit", &app_state.step_limit);
	ImGui::Checkbox("Enable Normal Visualization", &app_state.enable_normal_visualization);

	ImGui::End();
}

void render_gui() {

	// Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create ImGui windows
	create_main_window();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
	Window* window;
	try {
		window = new Window("Cloven", default_width, default_height);
	} catch (std::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}

	// Set window callbacks
	glfwSetKeyCallback(window->glfw_window, key_callback);
	glfwSetCursorPosCallback(window->glfw_window, cursor_position_callback);
	glfwSetScrollCallback(window->glfw_window, scroll_callback);
	glfwSetFramebufferSizeCallback(window->glfw_window, resize_callback);
	resize_callback(window->glfw_window, default_width, default_height);

	// Initialize GLEW
	GLenum glew_error = glewInit();
	if (glew_error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_error));
		return -1;
	}

	init_gui(window->glfw_window);

	camera = Camera();
	Shader shader("./shaders/shader.vert", "./shaders/test.frag");

	int nb_frames = 0;
	double current_time = glfwGetTime();
	double last_update_time = current_time; // When the FPS was last updated
	double last_frame_time = current_time; // When the last frame was rendered

	constexpr float quad_vertices[] = {
		-1.0f,  1.0f,
		-1.0f, -1.0f,
		 1.0f, -1.0f,
	
		-1.0f,  1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f
	};

	shader.bind();

	// Create VAO, VBO, and EBO
	unsigned int quad_vao, quad_vbo;
	glGenVertexArrays(1, &quad_vao);
	glGenBuffers(1, &quad_vbo);

	// Bind VAO and VBO
	glBindVertexArray(quad_vao);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);

	// Copy vertex data to VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	// Set vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
	glEnableVertexAttribArray(0);

	while (!window->should_close()) {

		// Update FPS
		{
			nb_frames++;
			current_time = glfwGetTime();
			app_state.update_delta_time = current_time - last_update_time;
			app_state.frame_delta_time = current_time - last_frame_time;
			last_frame_time = current_time;

			if (app_state.update_delta_time >= 1.0) {
				app_state.fps = nb_frames;
				nb_frames = 0;
				last_update_time += 1.0;
			}
		}

		window->render();

		if (glfwGetInputMode(window->glfw_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			camera.handle_keyboard_input(window->glfw_window, static_cast<float>(app_state.frame_delta_time));
		}

		{
			glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			// glDisable(GL_CULL_FACE);
			// glDisable(GL_DEPTH_TEST);
		}

		// Set uniforms
		{
			shader.bind();

			const glm::mat4 inverse_vp = glm::inverse(glm::perspective(glm::radians(camera.zoom), 1920.0f / 1080.0f, 0.1f, 100.0f) * camera.view_matrix());

			shader.set_uniform_mat4("u_inverse_projection_matrix", inverse_vp);
			shader.set_uniform_1f("u_time", glfwGetTime());
			shader.set_uniform_1i("u_max_iterations", app_state.max_iterations);
			shader.set_uniform_1i("u_escape_radius", app_state.escape_radius);
			shader.set_uniform_1i("u_step_limit", app_state.step_limit);
			shader.set_uniform_1f("u_power", app_state.power);
			shader.set_uniform_1f("u_epsilon", app_state.epsilon);
			shader.set_uniform_1f("u_max_distance", app_state.max_distance);
			shader.set_uniform_1f("u_ray_hit_threshold", app_state.ray_hit_threshold);
			shader.set_uniform_1i("u_enable_normal_visualization", app_state.enable_normal_visualization);
			//shader.set_uniform_1d("u_scale", 1.0);
			//shader.set_uniform_2d("u_offset", 0.0, 0.0);
			///shader.set_uniform_2d("u_trapping_point_offset", 0.001, 0.001);
			//shader.set_uniform_1i("u_palette_texture", 0);
			shader.set_uniform_vec3("u_camera_pos", camera.position);
			shader.set_uniform_2f("u_resolution", default_width, default_height);
		}

		// Draw quad
		glBindVertexArray(quad_vao);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		// glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		render_gui();
		window->update();
	}

	// Cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glDeleteVertexArrays(1, &quad_vao);
	glDeleteBuffers(1, &quad_vbo);

	return 0;
}
