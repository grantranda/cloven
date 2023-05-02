#pragma once

#include <GL/glew.h>
#include "glm/glm.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <GLFW/glfw3.h>

class Camera {
public:
	static constexpr float default_yaw = -90.0f;
	static constexpr float default_pitch = 0.0;
	static constexpr float default_speed = 0.01f;
	static constexpr float default_sensitivity = 0.5f;
	static constexpr float default_zoom = 90.0f;

	float yaw;
	float pitch;
	float speed;
	float sensitivity;
	float zoom;

	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 world_up;

	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = default_yaw,
	       float pitch = default_pitch);

	glm::mat4 view_matrix() const;
	void handle_keyboard_input(GLFWwindow* window, float delta_time);
	void handle_mouse_movement(float delta_x, float delta_y, GLboolean constrain_pitch = true);
	void handle_mouse_scroll(float delta_y);

private:
	void update_vectors();
};
