#pragma once

#include "../../event.h"

#include "keyboard.hpp"
#include "mouse.hpp"

#include <cstring>
#include <deque>

namespace os
{
namespace detail
{
namespace sdl
{
inline auto to_window_event_id(uint32_t id) -> window_event_id
{
	switch(id)
	{
		case SDL_EVENT_WINDOW_SHOWN:
			return window_event_id::shown;
		case SDL_EVENT_WINDOW_HIDDEN:
			return window_event_id::hidden;
		case SDL_EVENT_WINDOW_EXPOSED:
			return window_event_id::exposed;
		case SDL_EVENT_WINDOW_MOVED:
			return window_event_id::moved;
		case SDL_EVENT_WINDOW_RESIZED:
			return window_event_id::resized;
		case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
			return window_event_id::size_changed;
		case SDL_EVENT_WINDOW_MAXIMIZED:
			return window_event_id::maximized;
		case SDL_EVENT_WINDOW_MINIMIZED:
			return window_event_id::minimized;
		case SDL_EVENT_WINDOW_RESTORED:
			return window_event_id::restored;
		case SDL_EVENT_WINDOW_MOUSE_ENTER:
			return window_event_id::enter;
		case SDL_EVENT_WINDOW_MOUSE_LEAVE:
			return window_event_id::leave;
		case SDL_EVENT_WINDOW_FOCUS_GAINED:
			return window_event_id::focus_gained;
		case SDL_EVENT_WINDOW_FOCUS_LOST:
			return window_event_id::focus_lost;
		case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
			return window_event_id::close;
	}

	return window_event_id::none;
}

void fill_drop_event(event& ev, const SDL_Event& e)
{
	ev.drop.window_id = e.drop.windowID;
	ev.drop.x = e.drop.x;
	ev.drop.y = e.drop.y;
	if(e.drop.source != nullptr)
	{
		ev.drop.source = e.drop.source;
	}
	if(e.drop.data != nullptr)
	{
		ev.drop.data = e.drop.data;
	}
}

inline auto to_event(const SDL_Event& e) -> event
{


	event ev{};
	switch(e.type)
	{
		case SDL_EVENT_QUIT:
			ev.type = events::quit;
			break;
		case SDL_EVENT_TERMINATING:
			ev.type = events::app_terminating;
			break;
		case SDL_EVENT_LOW_MEMORY:
			ev.type = events::app_low_memory;
			break;
		case SDL_EVENT_WILL_ENTER_BACKGROUND:
			ev.type = events::app_will_enter_background;
			break;
		case SDL_EVENT_DID_ENTER_BACKGROUND:
			ev.type = events::app_did_enter_background;
			break;
		case SDL_EVENT_WILL_ENTER_FOREGROUND:
			ev.type = events::app_will_enter_foreground;
			break;
		case SDL_EVENT_DID_ENTER_FOREGROUND:
			ev.type = events::app_did_enter_foreground;
			break;
		case SDL_EVENT_DISPLAY_ORIENTATION:
			ev.type = events::display_orientation;
			break;
		case SDL_EVENT_DISPLAY_ADDED:
			ev.type = events::display_connected;
			break;
		case SDL_EVENT_DISPLAY_REMOVED:
			ev.type = events::display_disconnected;
			break;
		case SDL_EVENT_DISPLAY_MOVED:
			ev.type = events::display_moved;
			break;
		case SDL_EVENT_DISPLAY_CONTENT_SCALE_CHANGED:
			ev.type = events::display_content_scale_changed;
			break;
		case SDL_EVENT_KEY_DOWN:
			ev.type = events::key_down;
			ev.key.window_id = e.key.windowID;
			ev.key.code = detail::sdl::from_layout_independent_impl(e.key.scancode);
			ev.key.alt = (e.key.mod & SDL_KMOD_ALT) != 0;
			ev.key.ctrl = (e.key.mod & SDL_KMOD_CTRL) != 0;
			ev.key.shift = (e.key.mod & SDL_KMOD_SHIFT) != 0;
			ev.key.system = (e.key.mod & SDL_KMOD_GUI) != 0;
			break;
		case SDL_EVENT_KEY_UP:
			ev.type = events::key_up;
			ev.key.window_id = e.key.windowID;
			ev.key.code = detail::sdl::from_layout_independent_impl(e.key.scancode);
			ev.key.alt = (e.key.mod & SDL_KMOD_ALT) != 0;
			ev.key.ctrl = (e.key.mod & SDL_KMOD_CTRL) != 0;
			ev.key.shift = (e.key.mod & SDL_KMOD_SHIFT) != 0;
			ev.key.system = (e.key.mod & SDL_KMOD_GUI) != 0;
			break;
		case SDL_EVENT_TEXT_INPUT:
			ev.type = events::text_input;
			ev.text.window_id = e.text.windowID;
			ev.text.text = e.text.text;
			break;
		case SDL_EVENT_MOUSE_BUTTON_DOWN:
			ev.type = events::mouse_button;
			ev.button.window_id = e.button.windowID;
			ev.button.button = mouse::detail::sdl::from_impl(e.button.button);
			ev.button.state_id = state::pressed;
			ev.button.x = int32_t(e.button.x);
			ev.button.y = int32_t(e.button.y);
			ev.button.raw_x = int32_t(e.button.x);
			ev.button.raw_y = int32_t(e.button.y);
			break;
		case SDL_EVENT_MOUSE_BUTTON_UP:
			ev.type = events::mouse_button;
			ev.button.window_id = e.button.windowID;
			ev.button.button = mouse::detail::sdl::from_impl(e.button.button);
			ev.button.state_id = state::released;
			ev.button.x = int32_t(e.button.x);
			ev.button.y = int32_t(e.button.y);
			ev.button.raw_x = int32_t(e.button.x);
			ev.button.raw_y = int32_t(e.button.y);
			break;
		case SDL_EVENT_MOUSE_MOTION:
			ev.type = events::mouse_motion;
			ev.motion.window_id = e.motion.windowID;

			ev.motion.x = int32_t(e.motion.x);
			ev.motion.y = int32_t(e.motion.y);

			if (SDL_GetWindowRelativeMouseMode(SDL_GetWindowFromID(e.motion.windowID)))
			{
				auto& pos = mouse::detail::sdl::mouse_pos_while_relative();
				pos.x += e.motion.xrel;
				pos.y += e.motion.yrel;
				ev.motion.x = int32_t(pos.x);
				ev.motion.y = int32_t(pos.y);

			}

			ev.motion.raw_x = int32_t(ev.motion.x);
			ev.motion.raw_y = int32_t(ev.motion.y);

			break;
		case SDL_EVENT_MOUSE_WHEEL:
			ev.type = events::mouse_wheel;
			ev.wheel.window_id = e.wheel.windowID;
			ev.wheel.which = e.wheel.which;
			ev.wheel.x = static_cast<double>(e.wheel.x);
			ev.wheel.y = static_cast<double>(e.wheel.y);
			break;
		case SDL_EVENT_FINGER_DOWN:
			ev.type = events::finger_down;
			break;
		case SDL_EVENT_FINGER_UP:
			ev.type = events::finger_up;
			break;
		case SDL_EVENT_FINGER_MOTION:
			ev.type = events::finger_motion;
			break;
		case SDL_EVENT_CLIPBOARD_UPDATE:
			ev.type = events::clipboard_update;
			break;
		case SDL_EVENT_DROP_FILE:
            ev.type = events::drop_file;
			fill_drop_event(ev, e);
			break;
        case SDL_EVENT_DROP_TEXT:
            ev.type = events::drop_text;
			fill_drop_event(ev, e);
			break;
        case SDL_EVENT_DROP_BEGIN:
            ev.type = events::drop_begin;
			fill_drop_event(ev, e);
			break;
        case SDL_EVENT_DROP_COMPLETE:
            ev.type = events::drop_complete;
			fill_drop_event(ev, e);
			break;
        case SDL_EVENT_DROP_POSITION:
            ev.type = events::drop_position;
			fill_drop_event(ev, e);
			break;

		case SDL_EVENT_GAMEPAD_ADDED:
			ev.type = events::gamepad_added;
			ev.gamepad_device.which = e.gdevice.which;
			break;
		case SDL_EVENT_GAMEPAD_REMOVED:
			ev.type = events::gamepad_removed;
			ev.gamepad_device.which = e.gdevice.which;
			break;

		case SDL_EVENT_JOYSTICK_ADDED:
			ev.type = events::joystic_added;
			ev.joystick_device.which = e.jdevice.which;
			break;
		case SDL_EVENT_JOYSTICK_REMOVED:
			ev.type = events::joystic_removed;
			ev.joystick_device.which = e.jdevice.which;
			break;
		default:
			if(e.type >= SDL_EVENT_WINDOW_FIRST && e.type <= SDL_EVENT_WINDOW_LAST)
			{
				ev.type = events::window;
				ev.window.window_id = e.window.windowID;
				ev.window.type = to_window_event_id(e.type);
				ev.window.data1 = e.window.data1;
				ev.window.data2 = e.window.data2;
			}
			else
			{
				ev.type = events::unknown;
			}
	}

	return ev;
}

inline void pump_events() noexcept
{
	SDL_Event ev{};
	while(SDL_PollEvent(&ev) != 0)
	{
		auto e = to_event(ev);
		push_event(std::move(e));
	}
}
} // namespace sdl
} // namespace detail
} // namespace os
