#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 front, glm::vec3 up, glm::vec3 right, float yaw, float pitch)
	: yaw(yaw),
      pitch(pitch),
      speed(default_speed),
      sensitivity(default_sensitivity),
      zoom(default_zoom),
      position(position),
      front(front),
      up(up),
      right(right),
      world_up(up) {

}

glm::mat4 Camera::view_matrix() const {
	return lookAt(position, position + front, up);
}

void Camera::handle_keyboard_input(GLFWwindow* window, const float delta_time) {
    float velocity = speed * delta_time;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        velocity *= 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        velocity *= 2.5f;
    }

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        position += front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        position -= front * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        position -= right * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        position += right * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        position += up * velocity;
    }
    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        position -= up * velocity;
    }
}

void Camera::handle_mouse_movement(float delta_x, float delta_y, const GLboolean constrain_pitch) {
    delta_x *= sensitivity;
    delta_y *= sensitivity;

    yaw += delta_x;
    pitch += delta_y;

    if (constrain_pitch) {
	    if (pitch > 89.0f) {
            pitch = 89.0f;
	    }
        if (pitch < -89.0f) {
            pitch = -89.0f;
        }
    }

    update_vectors();
}

void Camera::handle_mouse_scroll(const float delta_y) {
    zoom -= delta_y;

    if (zoom < 1.0f) {
        zoom = 1.0f;
    }
    if (zoom > 120.0f) {
        zoom = 120.0f;
    }
}

void Camera::reset() {
    yaw = default_yaw;
    pitch = default_pitch;
    speed = default_speed;
    sensitivity = default_sensitivity;
    zoom = default_zoom;
	position[0] = 0.0f;
	position[1] = 0.0f;
	position[2] = 1.0f;
    front[0] = 0.0f;
	front[1] = 0.0f;
	front[2] = -1.0f;
    up[0] = 0.0f;
	up[1] = 1.0f;
	up[2] = 0.0f;
    right[0] = 0.0f;
	right[1] = 0.0f;
	right[2] = 0.0f;
    world_up[0] = up[0];
	world_up[1] = up[1];
	world_up[2] = up[2];
}

void Camera::update_vectors() {
    front = normalize(glm::vec3(
        cos(glm::radians(yaw)) * cos(glm::radians(pitch)),
        sin(glm::radians(pitch)),
        sin(glm::radians(yaw)) * cos(glm::radians(pitch))
    ));
    right = normalize(cross(front, world_up));
    up = normalize(cross(right, front));
}
