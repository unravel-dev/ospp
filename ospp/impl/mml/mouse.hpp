#pragma once
#include "../../mouse.h"
#include "window.hpp"

namespace os
{
namespace mouse
{
namespace detail
{
namespace mml
{
using window_impl = os::detail::mml::window_impl;
inline auto to_win_impl(const window& win) -> const window_impl&
{
	return *reinterpret_cast<window_impl*>(win.get_impl());
}
inline auto from_impl(::mml::mouse::button id) -> button
{
	switch(id)
	{
		case ::mml::mouse::button::left:
			return button::left;
		case ::mml::mouse::button::right:
			return button::right;
		case ::mml::mouse::button::middle:
			return button::middle;
		case ::mml::mouse::button::x_button1:
			return button::x1;
		case ::mml::mouse::button::x_button2:
			return button::x2;
		default:
			return button::none;
	}
}

inline auto to_impl(button b) -> ::mml::mouse::button
{
	switch(b)
	{
		case button::left:
			return ::mml::mouse::button::left;
		case button::right:
			return ::mml::mouse::button::right;
		case button::middle:
			return ::mml::mouse::button::middle;
		case button::x1:
			return ::mml::mouse::button::x_button1;
		case button::x2:
			return ::mml::mouse::button::x_button2;
		default:
			return ::mml::mouse::button::button_count;
	}
}

inline auto is_button_pressed(button b) noexcept -> bool
{
	auto impl_button = to_impl(b);
	return ::mml::mouse::is_button_pressed(impl_button);
}

inline auto get_position() noexcept -> point
{
	auto pos = ::mml::mouse::get_position();
	point result{};
	result.x = pos[0];
	result.y = pos[1];
	return result;
}
inline auto get_position(const window& relative_to) noexcept -> point
{
	auto pos = ::mml::mouse::get_position(to_win_impl(relative_to).get_impl());
	point result{};
	result.x = pos[0];
	result.y = pos[1];
	return result;
}

inline void set_position(const point& pos) noexcept
{
	::mml::mouse::set_position({{pos.x, pos.y}});
}

inline void set_position(const point& pos, const window& relative_to) noexcept
{
	::mml::mouse::set_position({{pos.x, pos.y}}, to_win_impl(relative_to).get_impl());
}

inline void capture(bool /*enabled*/)
{

}

inline void set_relative_mode(const window& /*relative_to*/, bool /*enabled*/) noexcept
{
}

inline auto get_relative_motion() noexcept -> point
{
	return {};
}

inline void disable(bool /*val*/)
{

}
} // namespace mml
} // namespace detail
} // namespace mouse
} // namespace os
