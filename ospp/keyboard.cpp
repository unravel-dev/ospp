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

void start_text_input() noexcept
{
	impl::start_text_input();
}

void stop_text_input() noexcept
{
	impl::stop_text_input();
}

auto is_text_input_active() noexcept -> bool
{
	return impl::is_text_input_active();
}
} // namespace os
