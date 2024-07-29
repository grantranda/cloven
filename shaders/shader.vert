#version 460 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 tex_coords;

out vec3 pos;
out vec3 v_ray_origin;
out vec3 v_ray_direction;

uniform mat4 u_inverse_view_matrix;
uniform mat4 u_inverse_projection_matrix;

void main() {
    pos = position;

    vec4 clip_space_pos = vec4(position.xy, 0.0, 1.0);
    vec4 view_space_pos = u_inverse_projection_matrix * clip_space_pos;
    vec4 world_space_pos = u_inverse_view_matrix * view_space_pos;

    world_space_pos /= world_space_pos.w;

    vec4 camera_world_pos = u_inverse_view_matrix * vec4(0.0, 0.0, 0.0, 1.0);

    v_ray_direction = normalize(world_space_pos.xyz - camera_world_pos.xyz);
    v_ray_origin = camera_world_pos.xyz;

    gl_Position = clip_space_pos;
}
