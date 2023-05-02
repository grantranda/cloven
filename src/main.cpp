#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "camera.h"
#include "gui.h"
#include "mesh.h"
#include "model.h"
#include "shader.h"
#include "vertex.h"
#include "window.h"
#include "windows/test_window.h"

int DEFAULT_WIDTH = 1920;
int DEFAULT_HEIGHT = 1080;

double delta_time = 0.0;
glm::mat4 inverseVP;
glm::vec2 screenSize = glm::vec2(DEFAULT_WIDTH, DEFAULT_HEIGHT);
GLfloat screenRatio = screenSize.x / screenSize.y;
Camera camera;

struct AppState {
	int max_iterations = 20;
	int escape_radius = 4;
	float power = 8.0f;
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

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.handle_mouse_scroll(static_cast<float>(yoffset));
}

void resizeCallback(GLFWwindow* win, int w, int h) {
	//std::cout << "\nresized to " << w << ", " << h << std::endl;
	glViewport(0, 0, w, h);
	screenSize.x = (GLfloat)w;
	screenSize.y = (GLfloat)h;
	screenRatio = screenSize.x / screenSize.y;
}

void init_gui(GLFWwindow* glfw_window) {
	auto glsl_version = "#version 460";

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

void create_test_window() {
	ImGui::Begin("Title");

	ImGui::InputInt("Max Iterations", &app_state.max_iterations);
	ImGui::InputInt("Escape Radius", &app_state.escape_radius);
	ImGui::InputFloat("Power", &app_state.power);

	ImGui::End();
}

void render_gui() {
	// Start the ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	// Create ImGui windows
	create_test_window();

	// Rendering
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

int main() {
	Window* window;
	try {
		window = new Window("Cloven", DEFAULT_WIDTH, DEFAULT_HEIGHT);
	} catch (std::exception& e) {
		fprintf(stderr, "%s\n", e.what());
		return -1;
	}

	camera = Camera();

	glfwSetKeyCallback(window->glfw_window, key_callback);
	glfwSetCursorPosCallback(window->glfw_window, cursor_position_callback);
	glfwSetScrollCallback(window->glfw_window, scroll_callback);

	glfwSetFramebufferSizeCallback(window->glfw_window, resizeCallback);

	GLenum glewError = glewInit();
	if (glewError != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(glewError));
		return -1;
	}

	init_gui(window->glfw_window);

	Shader shader("./shaders/shader.vert", "./shaders/test.frag");

	// Vertex vertices[] = {
	// 	Vertex(glm::vec3(-1.0, 1.0, 0.0), glm::vec2(0.0, 0.0)),
	// 	Vertex(glm::vec3(1.0, 1.0, 0.0), glm::vec2(1.0, 0.0)),
	// 	Vertex(glm::vec3(-1.0, -1.0, 0.0), glm::vec2(0.0, 1.0)),
	// 	Vertex(glm::vec3(1.0, -1.0, 0.0), glm::vec2(1.0, 1.0)),
	// };
	//
	// GLuint indices[] = {
	// 	0, 1, 3,
	// 	3, 1, 2
	// };

	float fov = 45.0f;
	float aspect = DEFAULT_WIDTH / DEFAULT_HEIGHT;
	float near = 0.1f;
	float far = 1000.0f;

	int nb_frames = 0;
	int fps;
	double last_time = 0.0;

	// Define vertices of the quad
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
	unsigned int quadVAO, quadVBO;
	glGenVertexArrays(1, &quadVAO);
	glGenBuffers(1, &quadVBO);

	// Bind VAO and VBO
	glBindVertexArray(quadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, quadVBO);

	// Copy vertex data to VBO
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_vertices), quad_vertices, GL_STATIC_DRAW);

	// Set vertex attributes
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	resizeCallback(window->glfw_window, DEFAULT_WIDTH, DEFAULT_HEIGHT);

	while (!window->should_close()) {
		{
			nb_frames++;
			float current_time = glfwGetTime();
			delta_time = current_time - last_time;
			last_time = current_time;
			// if (delta_time >= 1.0) {
			// 	fps = nb_frames;
			// 	nb_frames = 0;
			// 	last_time += 1.0;
			// }
		}

		inverseVP = glm::inverse(glm::perspective(glm::radians(camera.zoom), 1920.0f / 1080.0f, 0.1f, 100.0f) * camera.view_matrix());

		window->render();

		if (glfwGetInputMode(window->glfw_window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
			camera.handle_keyboard_input(window->glfw_window, static_cast<float>(delta_time));
		}

		{
			glClearColor(0.10f, 0.10f, 0.10f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			// glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			// glDisable(GL_CULL_FACE);
			// glDisable(GL_DEPTH_TEST);
		}

		{
			shader.bind();
			glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), aspect, near, far);

			shader.set_uniform_mat4("u_inverse_projection_matrix", inverseVP);
			shader.set_uniform_1f("u_time", glfwGetTime());
			shader.set_uniform_1i("u_max_iterations", app_state.max_iterations);
			shader.set_uniform_1i("u_escape_radius", app_state.escape_radius);
			shader.set_uniform_1f("u_power", app_state.power);
			shader.set_uniform_1d("u_scale", 1.0);
			shader.set_uniform_2d("u_offset", 0.0, 0.0);
			shader.set_uniform_2d("u_trapping_point_offset", 0.001, 0.001);
			shader.set_uniform_1i("u_palette_texture", 0);
			shader.set_uniform_vec3("u_camera_pos", camera.position);
			shader.set_uniform_2f("u_resolution", DEFAULT_WIDTH, DEFAULT_HEIGHT);
		}

		// Draw quad
		glBindVertexArray(quadVAO);
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

	glDeleteVertexArrays(1, &quadVAO);
	glDeleteBuffers(1, &quadVBO);

	return 0;
}
