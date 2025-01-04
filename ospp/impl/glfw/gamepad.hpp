#pragma once
#include "../../gamepad.h"
#include "window.hpp"

namespace os
{
namespace gamepad
{
namespace detail
{
namespace glfw
{

auto to_impl(const device_t& dev) -> int
{
	return static_cast<int>(dev.id);
}

auto open_device(id_t id) noexcept -> device_t
{
	if(glfwJoystickIsGamepad(id) == GLFW_FALSE)
	{
		return {};
	}

	device_t dev;
	dev.data = new id_t(id);
	dev.id = id;
	return dev;
}

void close_device(const device_t& dev) noexcept
{
	if(dev.data)
	{
		delete reinterpret_cast<id_t*>(dev.data);
	}
}

auto get_device_name(const device_t& dev) -> std::string
{
	return glfwGetGamepadName(to_impl(dev));
}

auto is_device_connected(const device_t& dev) -> bool
{
	return glfwJoystickPresent(to_impl(dev));
}

auto get_buttons_count(const device_t& dev) -> uint32_t
{
	return 15;
}

auto get_axis_count(const device_t& dev) -> uint32_t
{
	return 6;
}

auto get_button_state(const device_t& dev, uint32_t button_id) -> button_state
{
	GLFWgamepadstate state;
	if(glfwGetGamepadState(to_impl(dev), &state) == GLFW_TRUE)
	{
		auto val = state.buttons[button_id];
		return (val == GLFW_PRESS) ? button_state::pressed : button_state::released;
	}

	return button_state::released;
}

auto get_axis_value_normalized(const device_t& dev, uint32_t axis_id) -> float
{

	GLFWgamepadstate state;
	if(glfwGetGamepadState(to_impl(dev), &state) == GLFW_TRUE)
	{
		auto val = state.axes[axis_id];
		return val;
	}

	return 0.0f;
}

} // namespace glfw
} // namespace detail
} // namespace joystick
} // namespace os
