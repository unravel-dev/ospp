#include "keyboard.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/keyboard.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(MML_BACKEND)
#include "impl/mml/keyboard.hpp"
#define impl detail::mml
#elif defined(GLFW_BACKEND)
#include "impl/glfw/keyboard.hpp"
#define impl detail::glfw
#else
#error "unsupported backend"
#endif

namespace os
{
namespace key
{
auto from_string(const std::string& str) noexcept -> code
{
	return impl::key::from_string(str);
}

auto to_string(os::key::code key_code) noexcept -> std::string
{
	return impl::key::to_string(key_code);
}

auto is_pressed(os::key::code key_code) noexcept -> bool
{
	return impl::key::is_pressed(key_code);
}
} // namespace key

auto has_screen_keyboard() noexcept -> bool
{
	return impl::has_screen_keyboard();
}

void start_text_input(const window& win) noexcept
{
	impl::start_text_input(win);
}

void stop_text_input(const window& win) noexcept
{
	impl::stop_text_input(win);
}

auto is_text_input_active(const window& win) noexcept -> bool
{
	return impl::is_text_input_active(win);
}

void set_text_input_area(const window& win, const point& pos, const area& area, int cursor) noexcept
{
	impl::set_text_input_area(win, pos, area, cursor);
}
} // namespace os
