#pragma once

#include "keyboard.h"
#include "mouse.h"
#include <cstdint>
#include <functional>
#include <string>

namespace os
{
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
	/// The finger left without a release the application should act on (the system took the
	/// gesture, or the window lost it).
	finger_canceled,

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
	bool is_from_touch{};	/**< Made by the platform from a finger, which also reports finger events */
};

struct mouse_motion_event
{
	uint32_t window_id{};
	int32_t x{};	 /**< X coordinate, relative to window */
	int32_t y{};	 /**< Y coordinate, relative to window */
	int32_t raw_x{}; /**< X coordinate, raw */
	int32_t raw_y{}; /**< Y coordinate, raw */
	bool is_from_touch{}; /**< Made by the platform from a finger, which also reports finger events */
};

/// A finger on a touch device: finger_down, finger_up, finger_motion and finger_canceled.
struct touch_finger_event
{
	uint64_t touch_id{};  /**< The touch device */
	uint64_t finger_id{}; /**< The finger; unique on its device while it touches */
	uint32_t window_id{}; /**< The window under the finger */
	float x{};			  /**< X coordinate, relative to the window, normalized to 0...1 */
	float y{};			  /**< Y coordinate, relative to the window, normalized to 0...1 */
	float dx{};			  /**< X motion since the last event, normalized to -1...1 */
	float dy{};			  /**< Y motion since the last event, normalized to -1...1 */
	float pressure{};	  /**< Normalized to 0...1 */
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
		touch_finger_event finger;
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

/// Moves the platform's pending events into the queue that poll_event reads, without taking any.
/// On Android it blocks while the application is in the background.
void pump_events() noexcept;

/// Receives the application lifecycle events: app_terminating, app_low_memory and the
/// app_*_background and app_*_foreground events.
using app_event_callback = std::function<void(events)>;

/// Sets the function that receives application lifecycle events as soon as the platform reports
/// them, on the thread that pumps events and before poll_event returns them. On Android the event
/// pump blocks right after app_did_enter_background until the application returns, so work that
/// must happen before that (pausing audio, saving) belongs in this function. An empty function
/// removes it. Desktop platforms report no lifecycle events.
void set_app_event_callback(app_event_callback callback);
} // namespace os
