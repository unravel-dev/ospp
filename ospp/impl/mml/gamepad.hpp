#pragma once
#include "../../gamepad.h"
#include "window.hpp"

namespace os
{
namespace gamepad
{
namespace detail
{
namespace mml
{

auto to_impl(const device_t& dev) -> uint32_t
{
	return dev.id;
}

auto open_device(id_t id) noexcept -> device_t
{
	device_t dev;
	if(id < ::mml::joystick::count)
	{
		dev.data = new id_t(id);
	}
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
	return ::mml::joystick::get_identification(to_impl(dev)).name;
}

auto is_device_connected(const device_t& dev) -> bool
{
	return ::mml::joystick::is_connected(to_impl(dev));
}

auto get_buttons_count(const device_t& dev) -> uint32_t
{
	return ::mml::joystick::get_button_count(to_impl(dev));
}

auto get_axis_count(const device_t& dev) -> uint32_t
{
	return ::mml::joystick::axis_count;
}

auto get_button_state(const device_t& dev, uint32_t button_id) -> button_state
{
	bool val = ::mml::joystick::is_button_pressed(to_impl(dev), button_id);
	return val ? button_state::pressed : button_state::released;
}

auto get_axis_value_normalized(const device_t& dev, uint32_t axis_id) -> float
{
	auto axis = static_cast<::mml::joystick::axis>(axis_id);
	float raw = ::mml::joystick::get_axis_position(to_impl(dev), axis);

	float normalized = 0.0f;
	if(raw < 0)
		normalized = (float)raw / 100.0f;
	else
		normalized = (float)raw / 100.0f;

	return normalized;
}

} // namespace sdl
} // namespace detail
} // namespace joystick
} // namespace os
