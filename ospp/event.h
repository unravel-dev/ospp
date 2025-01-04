#pragma once

#include "keyboard.h"
#include "mouse.h"
#include <cstdint>
#include <string>

namespace os
{
// using finger = SDL_Finger;
// using touch_finger_event = SDL_TouchFingerEvent;

enum class window_event_id : uint8_t
{
	none = 0,
	shown,
	hidden,
	exposed,
	moved,
	resized,
	size_changed,
	minimized,
	maximized,
	restored,
	enter,
	leave,
	focus_gained,
	focus_lost,
	close
};

enum class events : uint8_t
{
	unknown = 0,
	quit,
	app_terminating,
	app_low_memory,
	app_will_enter_background,
	app_did_enter_background,
	app_will_enter_foreground,
	app_did_enter_foreground,

	window,
	key_down,
	key_up,

	text_input,

	mouse_wheel,

	mouse_button,
	mouse_motion,

	finger_down,
	finger_up,
	finger_motion,

	clipboard_update,
	drop_file,
	drop_text,
	drop_begin,
	drop_complete,
	drop_position,

	joystic_added,
	joystic_removed,

	gamepad_added,
	gamepad_removed,

	display_orientation,
	display_connected,
	display_disconnected,
	display_moved,
	display_content_scale_changed
};

enum class state : uint8_t
{
	none,
	pressed,
	released
};

struct quit_event
{
};

struct window_event
{
	uint32_t window_id{};
	int32_t data1{};
	int32_t data2{};
	window_event_id type{};
};

struct drop_event
{
	std::string source{}; /**< The source app that sent this drop event, or empty if that isn't available */
	std::string data{};
	uint32_t window_id{};
	float x{}; /**< X coordinate, relative to window (not on begin) */
	float y{}; /**< Y coordinate, relative to window (not on begin) */
};

struct text_input_event
{
	std::string text{}; /**< The input text (utf8)*/
	uint32_t window_id{};
};

struct mouse_button_event
{
	uint32_t window_id{};
	int32_t x{};			/**< X coordinate, relative to window */
	int32_t y{};			/**< Y coordinate, relative to window */
	int32_t raw_x{};		/**< X coordinate, raw */
	int32_t raw_y{};		/**< Y coordinate, raw */
	mouse::button button{}; /**< The mouse button index */
	state state_id{};		/**< ::pressed or ::released */
};

struct mouse_motion_event
{
	uint32_t window_id{};
	int32_t x{};	 /**< X coordinate, relative to window */
	int32_t y{};	 /**< Y coordinate, relative to window */
	int32_t raw_x{}; /**< X coordinate, raw */
	int32_t raw_y{}; /**< Y coordinate, raw */
};

struct mouse_wheel_event
{
	uint32_t window_id{};
	uint32_t which{}; /**< The mouse instance id */
	double x{};		  /**< The amount scrolled horizontally, positive to the right and negative to the left */
	double y{};		  /**< The amount scrolled vertically, positive away from the user */
};

struct key_event
{
	uint32_t window_id{};
	key::code code{}; /**< Code of the key that has been pressed */
	bool alt{};		  /**< Is the Alt key pressed? */
	bool ctrl{};	  /**< Is the Control key pressed? */
	bool shift{};	  /**< Is the Shift key pressed? */
	bool system{};	  /**< Is the System key pressed? */
};

struct display_event
{
};

struct joystick_device_event
{
	uint32_t which{}; /**< The joystick instance id */
};

struct gamepad_device_event
{
	uint32_t which{}; /**< The joystick instance id */
};

struct event
{
	drop_event drop;
	text_input_event text;
	union
	{
		mouse_wheel_event wheel;
		mouse_button_event button;
		window_event window;
		key_event key;
		mouse_motion_event motion;
		quit_event quit;
		display_event display;
		joystick_device_event joystick_device;
		gamepad_device_event gamepad_device;
	};
	events type;
};

void push_event(event&& e);
void push_event(const event& e);

auto poll_event(event& e) noexcept -> bool;
} // namespace os
