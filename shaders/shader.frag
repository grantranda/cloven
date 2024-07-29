#version 460 core

/*
MIT License for Simplex noise implementation by Ian McEwan and Ashima Arts:
https://github.com/ashima/webgl-noise/blob/master/LICENSE

Copyright (C) 2011 by Ashima Arts (Simplex noise)
Copyright (C) 2011-2016 by Stefan Gustavson (Classic noise and others)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

// The following code is derived from the Simplex noise implementation by Ian McEwan and Ashima Arts:

vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 mod289(vec4 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x) {
    return mod289(((x*34.0)+10.0)*x);
}

vec4 taylorInvSqrt(vec4 r) {
    return 1.79284291400159 - 0.85373472095314 * r;
}

float snoise(vec3 v) { 
    const vec2  C = vec2(1.0/6.0, 1.0/3.0) ;
    const vec4  D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    vec3 i  = floor(v + dot(v, C.yyy) );
    vec3 x0 =   v - i + dot(i, C.xxx) ;

    // Other corners
    vec3 g = step(x0.yzx, x0.xyz);
    vec3 l = 1.0 - g;
    vec3 i1 = min( g.xyz, l.zxy );
    vec3 i2 = max( g.xyz, l.zxy );

    //   x0 = x0 - 0.0 + 0.0 * C.xxx;
    //   x1 = x0 - i1  + 1.0 * C.xxx;
    //   x2 = x0 - i2  + 2.0 * C.xxx;
    //   x3 = x0 - 1.0 + 3.0 * C.xxx;
    vec3 x1 = x0 - i1 + C.xxx;
    vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    vec3 x3 = x0 - D.yyy;      // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i); 
    vec4 p = permute( permute( permute( 
             i.z + vec4(0.0, i1.z, i2.z, 1.0 ))
           + i.y + vec4(0.0, i1.y, i2.y, 1.0 )) 
           + i.x + vec4(0.0, i1.x, i2.x, 1.0 ));

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 = 294)
    float n_ = 0.142857142857; // 1.0/7.0
    vec3  ns = n_ * D.wyz - D.xzx;

    vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p,7*7)

    vec4 x_ = floor(j * ns.z);
    vec4 y_ = floor(j - 7.0 * x_ );    // mod(j,N)

    vec4 x = x_ *ns.x + ns.yyyy;
    vec4 y = y_ *ns.x + ns.yyyy;
    vec4 h = 1.0 - abs(x) - abs(y);

    vec4 b0 = vec4( x.xy, y.xy );
    vec4 b1 = vec4( x.zw, y.zw );

    //vec4 s0 = vec4(lessThan(b0,0.0))*2.0 - 1.0;
    //vec4 s1 = vec4(lessThan(b1,0.0))*2.0 - 1.0;
    vec4 s0 = floor(b0)*2.0 + 1.0;
    vec4 s1 = floor(b1)*2.0 + 1.0;
    vec4 sh = -step(h, vec4(0.0));

    vec4 a0 = b0.xzyw + s0.xzyw*sh.xxyy ;
    vec4 a1 = b1.xzyw + s1.xzyw*sh.zzww ;

    vec3 p0 = vec3(a0.xy,h.x);
    vec3 p1 = vec3(a0.zw,h.y);
    vec3 p2 = vec3(a1.xy,h.z);
    vec3 p3 = vec3(a1.zw,h.w);

    //Normalise gradients
    vec4 norm = taylorInvSqrt(vec4(dot(p0,p0), dot(p1,p1), dot(p2, p2), dot(p3,p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    vec4 m = max(0.5 - vec4(dot(x0,x0), dot(x1,x1), dot(x2,x2), dot(x3,x3)), 0.0);
    m = m * m;
    return 105.0 * dot( m*m, vec4( dot(p0,x0), dot(p1,x1), 
                                dot(p2,x2), dot(p3,x3) ) );
}

/*
The rest of the GLSL shader code in this file is licensed under the GNU General Public License v3.0.
*/

// Output
out vec4 frag_color;

// Constants
const int background_type_solid = 0;
const int background_type_dynamic = 1;
const int coloring_method_orbit_trap = 0;
const int coloring_method_distance_based = 1;

// Uniforms: General
uniform vec3 u_resolution;
uniform vec3 u_camera_pos;
uniform bool u_enable_normal_visualization;

// Uniforms: Fractal
uniform int u_max_iterations;
uniform int u_escape_radius;
uniform int u_step_limit;
uniform float u_max_distance;
uniform float u_power;
uniform float u_epsilon;
uniform float u_ray_hit_threshold;
uniform dvec2 u_trapping_point_offset;

// Uniforms: Coloring
uniform int u_coloring_method;
uniform int u_background_type;
uniform vec3 u_background_color;
uniform sampler1D u_gradient_texture;

// Uniforms: Lighting
uniform vec3 u_light_pos;
uniform float u_light_power;
uniform float u_noise_scale;
uniform float u_noise_amplitude;
uniform float u_ambient_strength;
uniform float u_diffuse_strength;
uniform float u_specular_strength;
uniform float u_specular_shininess;
uniform float u_shadow_softness;
uniform float u_shadow_min_distance;
uniform float u_shadow_min_step_size;
uniform float u_shadow_max_step_size;
uniform int u_shadow_max_iterations;
uniform float u_bloom_intensity_factor;
uniform vec3 u_bloom_color;
uniform float u_light_radius;
uniform vec3 u_light_color;
uniform bool u_show_light;
uniform bool u_apply_noise;
uniform bool u_apply_blinn_phong;
uniform bool u_apply_soft_shadow;
uniform bool u_apply_bloom;
uniform bool u_apply_ambient_occlusion;

// Input
in vec3 v_ray_origin;
in vec3 v_ray_direction;

// Global Variables
vec3 current_pos;
int current_steps;
bool exceeded_max_distance = false;
float orbit_trap_dist = 1e20;

// Function Prototypes
float sphere(vec3 pos, vec3 center, float radius);
float mandelbulb(vec3 pos, float power, int iterations);
float DE(vec3 pos, bool with_light);
float ray_march(vec3 ray_origin, vec3 ray_direction);
float soft_shadow(in vec3 ray_origin, float min_dist, float max_dist);
vec3 calculate_normal(vec3 pos);
vec3 blinn_phong(vec3 color, vec3 pos);
vec3 orbit_trap(float dist);
void main();

float sphere(vec3 pos, vec3 center, float radius) {
    return length(pos - center) - radius;
}

float mandelbulb(vec3 pos, float power, int iterations) {
	vec3 z = pos;
	float dr = 1.0;
	float r = 0.0;
    vec3 u_orbit_trap_center = vec3(0.0, 0.0, 0.0);
    float u_orbit_trap_radius = 0.5;

	for (int i = 0; i < iterations; i++) {
		r = length(z);
		if (r > u_escape_radius) break;

        float dist_to_trap = length(z - u_orbit_trap_center) - u_orbit_trap_radius;
        orbit_trap_dist = min(orbit_trap_dist, dist_to_trap);

        float theta = acos(z.z / r);
		float phi = atan(z.y, z.x);
		dr = pow(r, power - 1.0) * power * dr + 1.0;
		
		float zr = pow(r, power);
		theta = theta * power;
		phi = phi * power;
		
		z = zr * vec3(
            sin(theta) * cos(phi),
            sin(phi) * sin(theta),
            cos(theta)
        ) + pos;
	}
	return 0.5 * log(r) * r / dr;
}

float DE(vec3 pos, bool with_light) {
    float fractal_dist = mandelbulb(pos, u_power, u_max_iterations);

    if (u_show_light && with_light) {
        float light_dist = sphere(pos, u_light_pos, u_light_radius);
        return min(fractal_dist, light_dist);
    } else {
        return fractal_dist;
    }
}

float ray_march(vec3 ray_origin, vec3 ray_direction) {
	vec3 pos;
	float depth = 0.0;
	int i;

	for (i = 0; i < u_step_limit; i++) {
		pos = ray_origin + depth * ray_direction;
		float dist = DE(pos, true);
		depth += dist;

        if (depth > u_max_distance) {
            exceeded_max_distance = true;
            break;
        }
        if (u_background_type == background_type_dynamic) {
		    if ((dist < u_epsilon || dist > 20.0) && i > 2) break;
        } else if (dist < u_epsilon) {
            break;
        }
	}

	current_pos = pos;
    current_steps = i;
	return (1.0 - float(i) / u_step_limit);
}

float soft_shadow(in vec3 ray_origin, float min_dist, float max_dist) {
    vec3 ray_dir = normalize(u_light_pos - ray_origin);
    float result = 1.0;
    float current_dist = min_dist;
    float epsilon = 0.001;

    for(int i = 0; i < u_shadow_max_iterations && current_dist < max_dist; i++) {
        float surface_dist = DE(ray_origin + current_dist * ray_dir, false);

        if (surface_dist < epsilon) {
            result = 0.0;
            break;
        }

        result = min(result, u_shadow_softness * surface_dist / current_dist);
        current_dist += clamp(surface_dist, u_shadow_min_step_size, u_shadow_max_step_size);

        if (current_dist > max_dist) break;
    }

    result = max(result, -1.0);
    return 0.25 * (1.0 + result) * (1.0 + result) * (2.0 - result);
}

vec3 calculate_normal(vec3 pos) {
    float epsilon = 0.001;
    vec2 h = vec2(epsilon, 0.0);

    float dx = DE(pos + h.xyy, false) - DE(pos - h.xyy, false);
    float dy = DE(pos + h.yxy, false) - DE(pos - h.yxy, false);
    float dz = DE(pos + h.yyx, false) - DE(pos - h.yyx, false);

    return normalize(vec3(dx, dy, dz));
}

vec3 blinn_phong(vec3 color, vec3 pos) {
    const vec3 light_color = vec3(1.0, 1.0, 1.0);
    const vec3 spec_color = vec3(1.0, 1.0, 1.0);
    const float gamma = 2.2;

    vec3 light_dir = normalize(u_light_pos - pos);
    vec3 view_dir = normalize(u_camera_pos - pos);
    vec3 half_dir = normalize(light_dir + view_dir);
    vec3 normal = calculate_normal(pos);
    
    vec3 ambient = u_ambient_strength * color;

    float diff = max(dot(normal, light_dir), 0.0);
    vec3 diffuse = u_diffuse_strength * diff * color * light_color * u_light_power;

    float spec = pow(max(dot(normal, half_dir), 0.0), u_specular_shininess);
    vec3 specular = u_specular_strength * spec * spec_color * light_color * u_light_power;

    vec3 result = ambient + diffuse + specular;
    result = pow(result, vec3(1.0 / gamma));

    return result;
}

vec3 orbit_trap(float dist) {
    return texture(u_gradient_texture, clamp(dist, 0.0, 1.0)).rgb;
}

void main() {
    vec2 uv = gl_FragCoord.xy / u_resolution.xy;
    vec3 color;
    float ray_progress = ray_march(v_ray_origin, v_ray_direction);

    if (u_background_type == background_type_solid && (exceeded_max_distance || ray_progress < u_ray_hit_threshold)) {
        color = u_background_color;
    } else if (u_enable_normal_visualization) {
        vec3 dx = dFdx(current_pos);
        vec3 dy = dFdy(current_pos);
        vec3 surfaceNormal = normalize(cross(dx, dy));
        vec3 normalizedNormal = normalize(surfaceNormal);
        color = (normalizedNormal + 1.0) * 0.5;
    } else {
        float light_dist = sphere(current_pos, u_light_pos, u_light_radius);

        if (u_show_light && light_dist < u_epsilon) {
            color = u_light_color;
        } else {
            if (u_coloring_method == coloring_method_orbit_trap) {
                color = orbit_trap(orbit_trap_dist);
            } else {
                vec4 col = texture(u_gradient_texture, ray_progress);
                color = col.rgb;
            }
            if (u_apply_noise) {
                float noise_1 = snoise(current_pos * 2.0 * u_noise_scale) * u_noise_amplitude;
                float noise_2 = snoise(current_pos * 8.0 * u_noise_scale) * u_noise_amplitude;
                float noise = mix(noise_1, noise_2, 0.1) * u_noise_amplitude;
                color -= noise;
            }
            if (u_apply_blinn_phong) {
                color = blinn_phong(color, current_pos);
            }
            if (u_apply_soft_shadow) {
                color *= soft_shadow(current_pos, u_shadow_min_distance, length(u_light_pos - current_pos));
            }
            if (u_apply_bloom) {
                float bloom_intensity = exp(-ray_progress * u_bloom_intensity_factor);
                color = mix(color, u_bloom_color, bloom_intensity);
            }
            if (u_apply_ambient_occlusion) {
                color = mix(0.5 * color, color, ray_progress);
            }
        }
    }

    color = clamp(color, 0.0, 1.0);
    frag_color = vec4(color, 1.0);
}
