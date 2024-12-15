#pragma once
#include "../../mouse.h"
#include "window.hpp"

namespace os
{
namespace mouse
{
namespace detail
{
namespace sdl
{
using window_impl = os::detail::sdl::window_impl;
inline auto to_win_impl(const window& win) -> const window_impl&
{
	return *reinterpret_cast<window_impl*>(win.get_impl());
}

inline auto from_impl(uint8_t id) -> button
{
	switch(id)
	{
		case SDL_BUTTON_LEFT:
			return button::left;
		case SDL_BUTTON_RIGHT:
			return button::right;
		case SDL_BUTTON_MIDDLE:
			return button::middle;
		case SDL_BUTTON_X1:
			return button::x1;
		case SDL_BUTTON_X2:
			return button::x2;
		default:
			return button::none;
	}
}

inline auto to_impl(button b) -> uint8_t
{
	switch(b)
	{
		case button::left:
			return SDL_BUTTON_LEFT;
		case button::right:
			return SDL_BUTTON_RIGHT;
		case button::middle:
			return SDL_BUTTON_MIDDLE;
		case button::x1:
			return SDL_BUTTON_X1;
		case button::x2:
			return SDL_BUTTON_X2;
		default:
			return SDL_BUTTON_LEFT;
	}
}

inline auto is_button_pressed(button b) noexcept -> bool
{
	auto impl_button = to_impl(b);
	return (SDL_GetGlobalMouseState(nullptr, nullptr) & SDL_BUTTON_MASK(impl_button)) != 0;
}

inline auto get_position() noexcept -> point
{
	float x{};
	float y{};
	SDL_GetGlobalMouseState(&x, &y);
	return {int32_t(x), int32_t(y)};
}

inline auto get_position_impl(SDL_Window* relative_to) noexcept -> point
{
	point relative_pos{};
	auto global_pos = get_position();

	int window_x{};
	int window_y{};
	SDL_GetWindowPosition(relative_to, &window_x, &window_y);
	relative_pos.x = global_pos.x - window_x;
	relative_pos.y = global_pos.y - window_y;
	return relative_pos;
}

inline void set_position_impl(const point& pos, SDL_Window* relative_to) noexcept
{
	SDL_WarpMouseInWindow(relative_to, float(pos.x), float(pos.y));
}

inline point get_position(const window& relative_to) noexcept
{
	return get_position_impl(to_win_impl(relative_to).get_impl());
}

inline void set_position(const point& pos) noexcept
{
	SDL_WarpMouseGlobal(float(pos.x), float(pos.y));
}

inline void set_position(const point& pos, const window& relative_to) noexcept
{
	set_position_impl(pos, to_win_impl(relative_to).get_impl());
}

inline auto mouse_pos_while_relative() -> point&
{
	static point p{};
	return p;
}


inline auto mouse_pos_on_relative_start() -> point&
{
	static point p{};
	return p;
}

inline void capture(bool enabled) noexcept
{
	SDL_CaptureMouse(enabled);
}

inline void disable(bool val) noexcept
{
	// Get the window that currently has keyboard focus
	auto window = SDL_GetKeyboardFocus();

	if (window)
	{
		// Grab the input, confining the cursor to the window
		SDL_SetWindowMouseGrab(window, val);

		static bool relative = SDL_GetWindowRelativeMouseMode(window);

		bool is_different = relative != val;

		if(val && is_different)
		{
			auto mouse_window_pos = get_position_impl(window);
			mouse_pos_while_relative() = mouse_window_pos;
			mouse_pos_on_relative_start() = mouse_window_pos;
		}

		if(!val && is_different)
		{
			set_position_impl(mouse_pos_on_relative_start(), window);
		}

		relative = val;
	}

	// Set relative mouse mode
	SDL_SetWindowRelativeMouseMode(window, val);
}


} // namespace sdl
} // namespace detail
} // namespace mouse
} // namespace os
