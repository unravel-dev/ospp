#include "gamepad.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/gamepad.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(MML_BACKEND)
#include "impl/mml/gamepad.hpp"
#define impl detail::mml
#elif defined(GLFW_BACKEND)
#include "impl/glfw/gamepad.hpp"
#define impl detail::glfw
#else
#error "unsupported backend"
#endif

namespace os
{
namespace gamepad
{
auto open_device(id_t id) noexcept -> device_t
{
	return impl::open_device(id);
}

void close_device(const device_t& dev) noexcept
{
	impl::close_device(dev);
}

auto get_device_name(const device_t& dev) -> std::string
{
	return impl::get_device_name(dev);
}

auto is_device_connected(const device_t& dev) -> bool
{
	return impl::is_device_connected(dev);
}

auto get_buttons_count(const device_t& dev) -> uint32_t
{
	return impl::get_buttons_count(dev);
}

auto get_axis_count(const device_t& dev) -> uint32_t
{
	return impl::get_axis_count(dev);
}

auto get_button_state(const device_t& dev, uint32_t button_id) -> button_state
{
	return impl::get_button_state(dev, button_id);
}

auto get_axis_value_normalized(const device_t& dev, uint32_t axis_id) -> float
{
	return impl::get_axis_value_normalized(dev, axis_id);
}

} // namespace joystick
} // namespace os
