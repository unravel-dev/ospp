#include "mouse.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/mouse.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(MML_BACKEND)
#include "impl/mml/mouse.hpp"
#define impl detail::mml
#elif defined(GLFW_BACKEND)
#include "impl/glfw/mouse.hpp"
#define impl detail::glfw
#else
#error "unsupported backend"
#endif

namespace os
{
namespace mouse
{
auto is_button_pressed(button b) noexcept -> bool
{
	return impl::is_button_pressed(b);
}

auto get_position() noexcept -> point
{
	return impl::get_position();
}

auto get_position(const window& relative_to) noexcept -> point
{
	return impl::get_position(relative_to);
}

void set_position(const point& pos) noexcept
{
	impl::set_position(pos);
}

void set_position(const point& pos, const window& relative_to) noexcept
{
    impl::set_position(pos, relative_to);
}

void capture(bool enabled)
{
    impl::capture(enabled);
}

void disable(bool val)
{
	impl::disable(val);
}


} // namespace mouse
} // namespace os
