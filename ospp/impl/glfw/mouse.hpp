#pragma once
#include "../../mouse.h"
#include "window.hpp"

namespace os
{
namespace mouse
{
namespace detail
{
namespace glfw
{
using window_impl = os::detail::glfw::window_impl;
inline auto to_win_impl(const window& win) -> const window_impl&
{
	return *reinterpret_cast<window_impl*>(win.get_impl());
}
inline auto from_impl(int id) -> button
{
	switch(id)
	{
		case GLFW_MOUSE_BUTTON_LEFT:
			return button::left;
		case GLFW_MOUSE_BUTTON_RIGHT:
			return button::right;
		case GLFW_MOUSE_BUTTON_MIDDLE:
			return button::middle;
		case GLFW_MOUSE_BUTTON_4:
			return button::x1;
		case GLFW_MOUSE_BUTTON_5:
			return button::x2;
		default:
			return button::none;
	}
}
inline auto to_impl(button b) -> int
{
	switch(b)
	{
		case button::left:
			return GLFW_MOUSE_BUTTON_LEFT;
		case button::right:
			return GLFW_MOUSE_BUTTON_RIGHT;
		case button::middle:
			return GLFW_MOUSE_BUTTON_MIDDLE;
		case button::x1:
			return GLFW_MOUSE_BUTTON_4;
		case button::x2:
			return GLFW_MOUSE_BUTTON_5;
		default:
			return GLFW_MOUSE_BUTTON_LAST;
	}
}

inline auto is_button_pressed(button b) noexcept -> bool
{
	auto impl_button = to_impl(b);

	auto focused_win = os::detail::glfw::get_focused_win();
	if(focused_win)
	{
		return glfwGetKey(focused_win->get_impl(), impl_button) != GLFW_RELEASE;
	}
	return false;
}

inline auto get_position(const window& win) noexcept -> point
{
	point result{};
	double x{};
	double y{};
	glfwGetCursorPos(to_win_impl(win).get_impl(), &x, &y);
	result.x = static_cast<int32_t>(x);
	result.y = static_cast<int32_t>(y);
	return result;
}

inline auto get_position() noexcept -> point
{
	auto focused_win = os::detail::glfw::get_focused_win();
	if(focused_win)
	{
        auto window_pos = focused_win->get_position();
		point result{};
		double x{};
		double y{};
		glfwGetCursorPos(focused_win->get_impl(), &x, &y);
		result.x = window_pos.x + static_cast<int32_t>(x);
		result.y = window_pos.y + static_cast<int32_t>(y);
		return result;
	}
	return {};
}

inline void set_position(const point& pos, const window& win) noexcept
{
	glfwSetCursorPos(to_win_impl(win).get_impl(), static_cast<double>(pos.x), static_cast<double>(pos.y));
}
inline void set_position(const point& pos) noexcept
{
	auto focused_win = os::detail::glfw::get_focused_win();
	if(focused_win)
	{
        auto window_pos = focused_win->get_position();
		glfwSetCursorPos(focused_win->get_impl(), static_cast<double>(pos.x - window_pos.x), static_cast<double>(pos.y - window_pos.x));
	}
}

inline void capture(bool enabled)
{

}

inline void disable(bool enabled)
{
	auto focused_win = os::detail::glfw::get_focused_win();
	if(focused_win)
	{
		focused_win->grab_input(enabled);
	}
}
}
}
}
}
