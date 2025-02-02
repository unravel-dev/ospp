#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace os
{
namespace display
{
struct mode
{
	uint32_t w{};			 /**< width, in screen coordinates */
	uint32_t h{};			 /**< height, in screen coordinates */
	uint32_t refresh_rate{}; /**< refresh rate (or zero for unspecified) */
	uint32_t bpp{};			 /**< bits per pixel */
	float    display_scale{1.0}; /**< scale converting screen coordinates to pixels (e.g. a 2560x1440 screen size mode with 1.5 scale would have 3840x2160 pixels) */
};

struct bounds
{
	int x{};
	int y{};
	uint32_t w{};
	uint32_t h{};
};

auto enumerate_caps() -> void;
auto get_primary_display_index() -> int;
auto get_available_displays_count() -> int;
auto get_available_modes(int index = 0) -> std::vector<mode>;
auto get_desktop_mode(int index = 0) -> mode;
auto get_closest_mode(const mode& to, int index = 0) -> mode;
auto get_closest_mode(uint32_t width, uint32_t height, int index = 0) -> mode;
auto get_bounds(int index = 0) -> bounds;
auto get_usable_bounds(int index = 0) -> bounds;
auto get_content_scale(int index = 0) -> float;
auto to_string(const mode& m) -> std::string;
auto to_string(const bounds& b) -> std::string;
} // namespace display
} // namespace os
