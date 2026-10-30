#pragma once

#include "cursor.h"
#include "display_mode.h"
#include "types.hpp"

#include <cstdint>
#include <limits>
#include <memory>
#include <string>

namespace os
{
using native_handle = void*;
using native_display = void*;

class window
{
public:
	using ptr = std::shared_ptr<window>;
	using u_ptr = std::unique_ptr<window>;
	using weak_ptr = std::weak_ptr<window>;

	enum : uint32_t
	{
		fullscreen = 1 << 0,
		fullscreen_desktop = 1 << 1,
		hidden = 1 << 2,
		borderless = 1 << 3,
		resizable = 1 << 4,
		minimized = 1 << 5,
		maximized = 1 << 6,
		always_on_top = 1 << 7,
		no_taskbar = 1 << 8,
		tooltip = 1 << 9,
		popup = 1 << 10,
		utility = 1 << 11
	};
	constexpr static const auto centered = std::numeric_limits<int32_t>::max();

	window(const std::string& title, int32_t x, int32_t y, uint32_t w, uint32_t h, uint32_t flags = 0);
	window(const std::string& title, const point& pos, const area& size, uint32_t flags = 0);

	static auto is_any_focused() -> bool;

	auto get_native_handle() const -> native_handle;
	auto get_native_display() const -> native_display;

	auto get_display_scale() const -> float;

	/// GLFW-style HiDPI factor (per-window DPI / 96 on Windows; 1 on X11 until enhanced).
	auto get_content_scale() const -> float;

	auto is_open() const noexcept -> bool;
	auto get_id() const noexcept -> uint32_t;

	void set_size(uint32_t width, uint32_t height) noexcept;
	void set_size(const area& size) noexcept;
	auto get_size() const noexcept -> area;

	/// Client-area border sizes in window coordinates (top includes title bar). May be zero if unknown.
	auto get_frame_size() const noexcept -> frame_insets;

	/// How far in from each edge of the client area content stays clear of display cutouts, rounded
	/// corners and system bars, in window coordinates. Zero where nothing covers the window.
	auto get_safe_area_insets() const noexcept -> frame_insets;

	void set_maximum_size(uint32_t width, uint32_t height) noexcept;
	void set_maximum_size(const area& size) noexcept;
	auto get_maximum_size() const noexcept -> area;

	void set_minimum_size(uint32_t width, uint32_t height) noexcept;
	void set_minimum_size(const area& size) noexcept;
	auto get_minimum_size() const noexcept -> area;

	void set_position(int32_t x, int32_t y) noexcept;
	void set_position(const point& pos) noexcept;
	auto get_position() const noexcept -> point;

	auto get_title() const noexcept -> std::string;
	void set_title(const std::string& str) noexcept;

	void show() noexcept;
	void hide() noexcept;

	void maximize() noexcept;
	void minimize() noexcept;
	void restore() noexcept;
	void raise() noexcept;
    auto is_minimized() const noexcept -> bool;
    auto is_maximized() const noexcept -> bool;

	void set_border(bool b = true) noexcept;
	void set_fullscreen(bool b = true);
	auto is_fullscreen() const noexcept -> bool;

	/// The display the window is on, as an index of os::display.
	auto get_display_index() const noexcept -> int;

	/// The mode a fullscreen window switches its display to (exclusive fullscreen): the display's mode
	/// closest to \a mode. Returns false when the display has no fullscreen modes.
	auto set_fullscreen_mode(int display_index, const display::mode& mode) -> bool;

	/// A fullscreen window covers its display at the desktop's mode (borderless fullscreen), the default.
	void clear_fullscreen_mode();

	void set_opacity(float opacity);
	auto get_opacity() const -> float;

	void grab_input(bool grab) noexcept;
	auto is_input_grabbed() const noexcept -> bool;

	void request_focus();
	auto has_focus() const noexcept -> bool;
	void request_close() noexcept;

	void set_cursor(const cursor& c) noexcept;
	void set_cursor(cursor::type type) noexcept;
	void show_cursor(bool show) noexcept;

	//-----------------------------------------------------------------------------
	/// \a image pixels must be an array of \a width by \a height pixels
	/// in 32-bit RGBA format. If not, this will cause undefined behavior.
	/// If image pixels are null or either the width or the height are 0,
	/// the input is considered invalid.
	//-----------------------------------------------------------------------------
	void set_icon(const image& img);

	auto get_impl() const noexcept -> void*;

	static auto get_current_video_driver() -> const char*;
private:
	std::shared_ptr<void> impl_;
};


} // namespace os
