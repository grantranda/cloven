#pragma once

#include <vector>
#include <random>
#include <chrono>

#include <imgui.h>
#include <imgui_internal.h>

struct ColorStop {
	float position;
	ImVec4 color;
	bool selected;

	ColorStop(const float position, const ImVec4& color) : position(position), color(color), selected(false) {}

	bool operator==(const ColorStop& other) const {
		constexpr float epsilon = std::numeric_limits<float>::epsilon();
		return
			std::abs(color.x - other.color.x) < epsilon &&
			std::abs(color.y - other.color.y) < epsilon &&
			std::abs(color.z - other.color.z) < epsilon &&
			std::abs(color.w - other.color.w) < epsilon &&
			std::abs(position - other.position) < epsilon;
	}
};

class GradientEditor {
public:
	GradientEditor();

	[[nodiscard]] size_t get_num_stops() const;
	void handle_mouse_input(const ImVec2& preview_pos);
	void draw_stop(const ColorStop& stop, const ImVec2& preview_pos) const;
	void show();
	[[nodiscard]] ImVec4 interpolate(float position) const;
	[[nodiscard]] std::vector<unsigned char> generate_gradient() const;
	void set_default_gradient();
	void random_gradient();

private:
	ImVec2 preview_size = ImVec2(400, 40);
	ImVec2 stop_size = ImVec2(11.0f, 11.0f);
	int selected_stop_index = -1;
	bool is_dragging_stop = false;
	ColorStop* selected_stop = nullptr;
	std::vector<ColorStop> stops;

	static bool stop_comparator(const ColorStop& a, const ColorStop& b);
};
