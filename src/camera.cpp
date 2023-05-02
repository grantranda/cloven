#include "camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch) : speed(default_speed),
                                                                           sensitivity(default_sensitivity),
                                                                           zoom(default_zoom),
                                                                           front(glm::vec3(0.0f, 0.0f, -1.0f)) {
    this->position = position;
    this->world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;
}

glm::mat4 Camera::view_matrix() const {
	return lookAt(position, position + front, up);
}

void Camera::handle_keyboard_input(GLFWwindow* window, const float delta_time) {
    float velocity = speed * delta_time;

    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
        velocity = 0.001f * delta_time;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        velocity = 0.2f * delta_time;
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
    if (zoom > 90.0f) {
        zoom = 90.0f;
    }
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
