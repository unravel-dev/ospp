#pragma once
#include "../../event.h"

#include "keyboard.hpp"
#include "mouse.hpp"
#include "window.hpp"

#include <algorithm>
#include <codecvt>
#include <cstring>
#include <deque>
#include <locale>
namespace os
{
namespace detail
{
namespace mml
{

inline auto to_event(const ::mml::platform_event& e, uint32_t window_id) -> event
{
	event ev{};
	switch(e.type)
	{
		case ::mml::platform_event::closed:
			ev.type = events::window;
			ev.window.window_id = window_id;
			ev.window.type = window_event_id::close;
			break;
		case ::mml::platform_event::resized:
			ev.type = events::window;
			ev.window.window_id = window_id;
			ev.window.type = window_event_id::resized;
			ev.window.data1 = static_cast<int32_t>(e.size.width);
			ev.window.data2 = static_cast<int32_t>(e.size.height);
			break;
		case ::mml::platform_event::moved:
			ev.type = events::window;
			ev.window.window_id = window_id;
			ev.window.type = window_event_id::moved;
			ev.window.data1 = static_cast<int32_t>(e.move.x);
			ev.window.data2 = static_cast<int32_t>(e.move.y);
			break;
		case ::mml::platform_event::lost_focus:
			ev.type = events::window;
			ev.window.window_id = window_id;
			ev.window.type = window_event_id::focus_lost;
			break;
		case ::mml::platform_event::gained_focus:
			ev.type = events::window;
			ev.window.window_id = window_id;
			ev.window.type = window_event_id::focus_gained;
			break;
		case ::mml::platform_event::mouse_entered:
			ev.type = events::window;
			ev.window.window_id = window_id;
			ev.window.type = window_event_id::enter;
			break;
		case ::mml::platform_event::mouse_left:
			ev.type = events::window;
			ev.window.window_id = window_id;
			ev.window.type = window_event_id::leave;
			break;
		case ::mml::platform_event::key_pressed:
			ev.type = events::key_down;
			ev.key.window_id = window_id;
			ev.key.code = detail::mml::from_layout_independent_impl(e.key.scancode);
			ev.key.alt = e.key.alt;
			ev.key.ctrl = e.key.control;
			ev.key.shift = e.key.shift;
			ev.key.system = e.key.system;
			break;
		case ::mml::platform_event::key_released:
			ev.type = events::key_up;
			ev.key.window_id = window_id;
			ev.key.code = detail::mml::from_layout_independent_impl(e.key.scancode);
			ev.key.alt = e.key.alt;
			ev.key.ctrl = e.key.control;
			ev.key.shift = e.key.shift;
			ev.key.system = e.key.system;
			break;

		case ::mml::platform_event::text_entered:
			if(e.text.unicode < 32 || (e.text.unicode > 126 && e.text.unicode < 160))
				break;

			ev.type = events::text_input;
			ev.text.window_id = window_id;
			ev.text.text = std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>>{}.to_bytes(e.text.unicode);
			break;
		case ::mml::platform_event::mouse_button_pressed:
			ev.type = events::mouse_button;
			ev.button.window_id = window_id;
			ev.button.button = mouse::detail::mml::from_impl(e.mouse_button.button);
			ev.button.state_id = state::pressed;
			ev.button.x = e.mouse_button.x;
			ev.button.y = e.mouse_button.y;
			ev.button.raw_x = e.mouse_button.x;
			ev.button.raw_y = e.mouse_button.y;
			break;
		case ::mml::platform_event::mouse_button_released:
			ev.type = events::mouse_button;
			ev.button.window_id = window_id;
			ev.button.button = mouse::detail::mml::from_impl(e.mouse_button.button);
			ev.button.state_id = state::released;
			ev.button.x = e.mouse_button.x;
			ev.button.y = e.mouse_button.y;
			ev.button.raw_x = e.mouse_button.x;
			ev.button.raw_y = e.mouse_button.y;
			break;
		case ::mml::platform_event::mouse_moved:
			ev.type = events::mouse_motion;
			ev.motion.window_id = window_id;
			ev.motion.x = e.mouse_move.x;
			ev.motion.y = e.mouse_move.y;
			ev.motion.raw_x = e.mouse_move.x;
			ev.motion.raw_y = e.mouse_move.y;
			break;
		case ::mml::platform_event::mouse_wheel_scrolled:
			ev.type = events::mouse_wheel;
			ev.wheel.window_id = window_id;
			ev.wheel.which = static_cast<uint32_t>(e.mouse_wheel_scroll.wheel);
			switch(e.mouse_wheel_scroll.wheel)
			{
				case ::mml::mouse::horizontal_wheel:
					ev.wheel.x = static_cast<double>(e.mouse_wheel_scroll.delta);
					break;
				case ::mml::mouse::vertical_wheel:
					ev.wheel.y = static_cast<double>(e.mouse_wheel_scroll.delta);
					break;
			}
			break;
		case ::mml::platform_event::joystick_connected:
			ev.type = events::joystic_added;
			ev.joystick_device.which = e.joystick_connect.joystick_id;
			break;
		case ::mml::platform_event::joystick_disconnected:
			ev.type = events::joystic_removed;
			ev.joystick_device.which = e.joystick_connect.joystick_id;
			break;
		case ::mml::platform_event::touch_began:
			ev.type = events::finger_down;
			break;
		case ::mml::platform_event::touch_ended:
			ev.type = events::finger_up;
			break;
		case ::mml::platform_event::touch_moved:
			ev.type = events::finger_motion;
			break;

		default:
			ev.type = events::unknown;
	}

	return ev;
}

inline void pump_events() noexcept
{
	auto& windows = get_windows();
	for(auto& window : windows)
	{
		auto& win_impl = window->get_impl();

		::mml::platform_event ev{};
		if(win_impl.poll_event(ev))
		{
			if(ev.type == ::mml::platform_event::closed)
			{
				window->set_recieved_close_event(true);
			}
			auto e = to_event(ev, window->get_id());

			push_event(std::move(e));
		}
	}

	static bool reported{};
	if(!reported)
	{
		auto all_closed = std::all_of(std::begin(windows), std::end(windows),
									  [](const auto& e) { return e->recieved_close_event(); });
		if(all_closed)
		{
			reported = true;

			event ev{};
			ev.type = events::quit;
			push_event(std::move(ev));
		}
	}
}
} // namespace mml
} // namespace detail
} // namespace os
