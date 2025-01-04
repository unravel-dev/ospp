#pragma once
#include "../../gamepad.h"
#include "window.hpp"

namespace os
{
namespace gamepad
{
namespace detail
{
namespace sdl
{

auto to_impl(const device_t& dev) -> SDL_Gamepad*
{
	return reinterpret_cast<SDL_Gamepad*>(dev.data);
}

auto open_device(id_t id) noexcept -> device_t
{
	device_t dev;
	dev.data = SDL_OpenGamepad(id);
	dev.id = id;
	return dev;
}

auto get_device_name(const device_t& dev) -> std::string
{
	return SDL_GetGamepadName(to_impl(dev));
}

void close_device(const device_t& dev) noexcept
{
	SDL_CloseGamepad(to_impl(dev));
}

auto is_device_connected(const device_t& dev) -> bool
{
	return SDL_GamepadConnected(to_impl(dev));
}

auto get_buttons_count(const device_t& dev) -> uint32_t
{
	return SDL_GAMEPAD_BUTTON_COUNT;
}

auto get_axis_count(const device_t& dev) -> uint32_t
{
	return SDL_GAMEPAD_AXIS_COUNT;
}

auto get_button_state(const device_t& dev, uint32_t button_id) -> button_state
{
	SDL_GamepadButton button = static_cast<SDL_GamepadButton>(button_id);
	Uint8 val = SDL_GetGamepadButton(to_impl(dev), button);

	return (val == 1) ? button_state::pressed : button_state::released;
}

auto get_axis_value_normalized(const device_t& dev, uint32_t axis_id) -> float
{
	SDL_GamepadAxis sdlAxis = static_cast<SDL_GamepadAxis>(axis_id);
	Sint16 raw = SDL_GetGamepadAxis(to_impl(dev), sdlAxis);

	float normalized = 0.0f;
	if(raw < 0)
		normalized = (float)raw / 32768.0f;
	else
		normalized = (float)raw / 32767.0f;

	return normalized;
}

} // namespace sdl
} // namespace detail
} // namespace joystick
} // namespace os
