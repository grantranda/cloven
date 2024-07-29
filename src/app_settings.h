#pragma once

#include <glm/vec3.hpp>

// Defaults
constexpr int default_width = 1920;
constexpr int default_height = 1080;
constexpr int gui_width = 460;
constexpr int default_max_iterations = 25;
constexpr int default_escape_radius = 4;
constexpr int default_step_limit = 1000;
constexpr float default_power = 8.0f;
constexpr float default_epsilon = 0.0001f;
constexpr float default_max_distance = 50.0f;
constexpr float default_ray_hit_threshold = 0.00001f;
constexpr float default_background_color[3] = {1.0f, 1.0f, 1.0f};
constexpr float default_light_pos[3] = {2.0f, 2.0f, 5.0f};
constexpr float default_light_power = 0.4f;
constexpr float default_light_radius = 0.1f;
constexpr float default_light_color[3] = {0.9f, 0.9f, 0.9f};
constexpr float default_noise_scale = 1.0f;
constexpr float default_noise_amplitude = 0.5f;
constexpr float default_ambient_strength = 0.5f;
constexpr float default_diffuse_strength = 1.0f;
constexpr float default_specular_strength = 0.5f;
constexpr float default_specular_shininess = 32.0f;
constexpr float default_shadow_softness = 32.0f;
constexpr float default_shadow_min_distance = 0.01f;
constexpr float default_shadow_min_step_size = 0.01f;
constexpr float default_shadow_max_step_size = 0.1f;
constexpr int default_shadow_max_iterations = 128;
constexpr float default_bloom_intensity_factor = 5.0f;
constexpr float default_bloom_color[3] = {1.0f, 1.0f, 1.0f};
constexpr float default_camera_pos[3] = {0.0f, 0.0f, 1.0f};

struct AppSettings {
	// Rendering settings
	int max_iterations = default_max_iterations;
	int escape_radius = default_escape_radius;
	int step_limit = default_step_limit;
	float power = default_power;
	float epsilon = default_epsilon;
	float max_distance = default_max_distance;
	float ray_hit_threshold = default_ray_hit_threshold;
	int coloring_method = 0;
	int background_type = 0;
	float background_color[3] = {default_background_color[0], default_background_color[1], default_background_color[2]};
	glm::vec3 light_pos = glm::vec3(default_light_pos[0], default_light_pos[1], default_light_pos[2]);
	float light_power = default_light_power;
	float light_radius = default_light_radius;
	float light_color[3] = {default_light_color[0], default_light_color[1], default_light_color[2]};
	float noise_scale = default_noise_scale;
	float noise_amplitude = default_noise_amplitude;
	float ambient_strength = default_ambient_strength;
	float diffuse_strength = default_diffuse_strength;
	float specular_strength = default_specular_strength;
	float specular_shininess = default_specular_shininess;
	float shadow_softness = default_shadow_softness;
	float shadow_min_distance = default_shadow_min_distance;
	float shadow_min_step_size = default_shadow_min_step_size;
	float shadow_max_step_size = default_shadow_max_step_size;
	int shadow_max_iterations = default_shadow_max_iterations;
	float bloom_intensity_factor = default_bloom_intensity_factor;
	float bloom_color[3] = {default_bloom_color[0], default_bloom_color[1], default_bloom_color[2]};
	bool show_light = false;
	bool apply_noise = true;
	bool apply_blinn_phong = true;
	bool apply_soft_shadow = true;
	bool apply_bloom = true;
	bool apply_ambient_occlusion = true;
	bool enable_normal_visualization = false;

	// GUI settings
	bool show_gui = true;
	bool show_gradient_editor = false;
	int fps = 0;
	double update_delta_time = 0.0;
	double frame_delta_time = 0.0;
};
