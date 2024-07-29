#include "gradient_editor.h"

GradientEditor::GradientEditor() {
	set_default_gradient();
}

size_t GradientEditor::get_num_stops() const {
	return stops.size();
}

void GradientEditor::handle_mouse_input(const ImVec2& preview_pos) {
    const ImGuiIO& io = ImGui::GetIO();
    const ImVec2 mouse_pos = io.MousePos;

    // Selecting a stop
    if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
        for (size_t i = 0; i < stops.size(); ++i) {
            const ImVec2 stop_pos(preview_pos.x + stops[i].position * preview_size.x - 5, preview_pos.y - 18);

            if (ImGui::IsMouseHoveringRect(stop_pos, ImVec2(stop_pos.x + stop_size.x, stop_pos.y + stop_size.y))) {
            	selected_stop_index = static_cast<int>(i);
                selected_stop = &stops[i];
				is_dragging_stop = true;
                break;
            }
        }
    }

    // Dragging a stop
    if (ImGui::IsMouseDragging(ImGuiMouseButton_Left) && selected_stop_index != -1) {
        const ImVec2 stop_pos(preview_pos.x + stops[selected_stop_index].position * preview_size.x - 5, preview_pos.y - 18);

        if (is_dragging_stop || ImGui::IsMouseHoveringRect(stop_pos, ImVec2(stop_pos.x + stop_size.x, stop_pos.y + stop_size.y))) {
	        const float position = (mouse_pos.x - preview_pos.x) / preview_size.x;
	        stops[selected_stop_index].position = std::clamp(position, 0.0f, 1.0f);
	        is_dragging_stop = true;
        }
    }

    if (!is_dragging_stop) {

    	// Adding a stop
	    if (ImGui::IsMouseReleased(0)) {
	        if (ImGui::IsMouseHoveringRect(preview_pos, ImVec2(preview_pos.x + preview_size.x, preview_pos.y + preview_size.y))) {
	            float position = (mouse_pos.x - preview_pos.x) / preview_size.x;
	            stops.emplace_back(position, interpolate(position));
	            std::ranges::sort(stops, stop_comparator);

	            for (size_t i = 0; i < stops.size(); i++) {
		            if (std::abs(stops[i].position - position) < FLT_EPSILON) {
                        selected_stop_index = static_cast<int>(i);
						selected_stop = &stops[i];
	                    break;
		            }
	            }
	        }
	    }

        // Deleting a stop
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            for (size_t i = 0; i < stops.size(); ++i) {
	            const ImVec2 stop_pos(preview_pos.x + stops[i].position * preview_size.x - 5, preview_pos.y - 18);

	            if (ImGui::IsMouseHoveringRect(stop_pos, ImVec2(stop_pos.x + stop_size.x, stop_pos.y + stop_size.y))) {
                    if (get_num_stops() > 1) {
	                    stops.erase(stops.begin() + static_cast<int>(i));

	                    if (!stops.empty()) {
		                    selected_stop_index = 0;
							selected_stop = &stops.front();
	                    } else {
		                    selected_stop_index = -1;
							selected_stop = nullptr;
	                    }
                    }
	                break;
	            }
	        }
        }
    }

    if (ImGui::IsMouseReleased(0)) {
    	is_dragging_stop = false;
    }
}

void GradientEditor::draw_stop(const ColorStop& stop, const ImVec2& preview_pos) const {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const ImVec2 stop_pos(preview_pos.x + stop.position * preview_size.x - 5, preview_pos.y - 18);

    // Draw stop triangle
    const ImVec2 vertices[3] = {
        ImVec2(stop_pos.x + 5.5f, stop_pos.y + 17), // B
        ImVec2(stop_pos.x + 2.5f, stop_pos.y + 11), // TL
        ImVec2(stop_pos.x + 8.5f, stop_pos.y + 11) // TR
    };
    draw_list->AddPolyline(vertices, 3, IM_COL32(255, 255, 255, 255), true, 1.0f);

    // Draw outer black border
    draw_list->AddRect(
        stop_pos, ImVec2(stop_pos.x + stop_size.x, stop_pos.y + stop_size.y),
        IM_COL32(0, 0, 0, 255),
        0.0f, 0, 1.0f
    );

    // Draw inner white border
    draw_list->AddRect(
        ImVec2(stop_pos.x + 1, stop_pos.y + 1),
        ImVec2(stop_pos.x + stop_size.x - 1, stop_pos.y + stop_size.y - 1),
        IM_COL32(255, 255, 255, 255),
        0.0f, 0, 1.0f
    );

    // Draw stop color
    draw_list->AddRectFilled(
        ImVec2(stop_pos.x + 2, stop_pos.y + 2),
        ImVec2(stop_pos.x + stop_size.x - 2, stop_pos.y + stop_size.y - 2),
        ImGui::ColorConvertFloat4ToU32(stop.color)
    );

    // Draw selected stop border
    if (selected_stop && stop == *selected_stop) {
        draw_list->AddRect(stop_pos, ImVec2(stop_pos.x + stop_size.x, stop_pos.y + stop_size.y), IM_COL32(255, 255, 255, 255));

        const ImVec2 selected_rect_pos(stop_pos.x - 1, stop_pos.y - 1);
        const ImVec2 selected_rect_size(stop_size.x + 2, stop_size.y + 2);

        draw_list->AddRect(
            selected_rect_pos, 
            ImVec2(selected_rect_pos.x + selected_rect_size.x, selected_rect_pos.y + selected_rect_size.y), 
            IM_COL32(0, 0, 0, 255)
        );
    }
}

void GradientEditor::show() {
	const ImVec2 cursor_screen_pos = ImGui::GetCursorScreenPos();
	const ImVec2 preview_pos(cursor_screen_pos.x + 10, cursor_screen_pos.y + 20);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Preview
    for (int x = 0; x < static_cast<int>(preview_size.x); ++x) {
        const auto x_float = static_cast<float>(x);
	    const float position = x_float / preview_size.x;
	    const ImVec2 start(preview_pos.x + x_float, preview_pos.y);
	    const ImVec2 end(preview_pos.x + x_float + 1, preview_pos.y + preview_size.y);
	    const ImVec4 color = interpolate(position);

        draw_list->AddRectFilled(
            start,
            end,
            IM_COL32(
	            static_cast<int>(color.x * 255),
	            static_cast<int>(color.y * 255),
	            static_cast<int>(color.z * 255),
	            static_cast<int>(color.w * 255)
            )
        );
    }

    // Stops
    for (auto stop : stops) {
    	draw_stop(stop, preview_pos);
    }

    // Preview border
    draw_list->AddRect(
        preview_pos,
        ImVec2(preview_pos.x + preview_size.x, preview_pos.y + preview_size.y),
        IM_COL32(128, 128, 128, 255),
        0.0f,
        ImDrawFlags_None,
        1.0f
    );

    ImGui::Dummy(ImVec2(preview_size.x + 10, preview_size.y + 25));

    // Color Picker
    if (selected_stop) {
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10);
        ImGui::SetNextItemWidth(250.0f);
        ImGui::BeginChild("ColorPicker", ImVec2(250.0f, 250.0f), false, ImGuiWindowFlags_NoScrollbar);
		ImGui::ColorPicker3("Color", (float*)&selected_stop->color, ImGuiColorEditFlags_DisplayRGB | ImGuiColorEditFlags_NoSidePreview);
    	ImGui::EndChild();
    }

    handle_mouse_input(preview_pos);
}

ImVec4 GradientEditor::interpolate(const float position) const {
    if (stops.empty()) {
        return {0, 0, 0, 1};
    }

    std::vector<ColorStop> sorted_stops = stops;
    std::ranges::sort(sorted_stops, stop_comparator);

    if (position < sorted_stops.front().position) {
	    return sorted_stops.front().color;
    }
	if (position > sorted_stops.back().position) {
		return sorted_stops.back().color;
    }

    for (size_t i = 0; i < sorted_stops.size() - 1; i++) {
        const ColorStop& current = sorted_stops[i];
        const ColorStop& next = sorted_stops[i + 1];

        if (position >= current.position && position <= next.position) {
            const float t = (position - current.position) / (next.position - current.position);
            return {
                ImLerp(current.color.x, next.color.x, t),
                ImLerp(current.color.y, next.color.y, t),
                ImLerp(current.color.z, next.color.z, t),
                ImLerp(current.color.w, next.color.w, t)
            };
        }
    }
    
    return sorted_stops.back().color;
}

std::vector<unsigned char> GradientEditor::generate_gradient() const {
	std::vector<unsigned char> gradient;

	for (int i = 0; i < 256; i++) {
		const float position = static_cast<float>(i) / 255.0f;
		const ImVec4 color = interpolate(position);

		gradient.push_back(static_cast<unsigned char>(color.x * 255.0f));
		gradient.push_back(static_cast<unsigned char>(color.y * 255.0f));
		gradient.push_back(static_cast<unsigned char>(color.z * 255.0f));
	}

	return gradient;
}

void GradientEditor::set_default_gradient() {
    stops.clear();
	stops.emplace_back(0, ImVec4(0, 0, 0, 1.0));
	stops.emplace_back(0.5, ImVec4(0.5, 0.5, 0.5, 1.0));
	stops.emplace_back(1.0, ImVec4(1.0, 1.0, 1.0, 1.0));
    selected_stop_index = 1;
    selected_stop = &stops.front();
}

void GradientEditor::random_gradient() {
    const int num_stops = static_cast<int>(get_num_stops());
    std::default_random_engine generator(std::chrono::system_clock::now().time_since_epoch().count());
    std::uniform_int_distribution distribution(0, 255);
    stops.clear();

    for (int i = 0; i < num_stops; ++i) {
        const float position = static_cast<float>(distribution(generator)) / 255.0f;
        auto color = ImVec4(
            static_cast<float>(distribution(generator)) / 255.0f,
            static_cast<float>(distribution(generator)) / 255.0f,
            static_cast<float>(distribution(generator)) / 255.0f,
            1.0f
        );

        stops.emplace_back(position, color);
    }

	std::ranges::sort(stops, stop_comparator);
    selected_stop_index = 1;
    selected_stop = &stops.front();
}

bool GradientEditor::stop_comparator(const ColorStop& a, const ColorStop& b) {
	return a.position < b.position;
}
