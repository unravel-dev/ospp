#pragma once

#include "types.hpp"
#include <string>

namespace os
{

namespace gamepad
{
using id_t = int;

struct device_t
{
	id_t id{};
	void* data{};
};

enum button_state
{
	pressed,
	released
};

auto open_device(id_t) noexcept -> device_t;

void close_device(const device_t& dev) noexcept;

auto get_device_name(const device_t& dev) -> std::string;

auto is_device_connected(const device_t& dev) -> bool;

auto get_buttons_count(const device_t& dev) -> uint32_t;

auto get_axis_count(const device_t& dev) -> uint32_t;

auto get_button_state(const device_t& dev, uint32_t button_id) -> button_state;

auto get_axis_value_normalized(const device_t& dev, uint32_t axis_id) -> float;

} // namespace joystick
} // namespace os
