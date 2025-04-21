#include "init.h"
#include "cursor.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/init.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(MML_BACKEND)
#include "impl/mml/init.hpp"
#define impl detail::mml
#elif defined(GLFW_BACKEND)
#include "impl/glfw/init.hpp"
#define impl detail::glfw
#else
#error "unsupported backend"
#endif

namespace os
{
void shutdown() noexcept
{
	system_cursors().clear();
	impl::shutdown();
}

auto init() -> bool
{
    return impl::init();
}

auto get_current_backend() -> const char*
{
    return impl::get_current_backend();
}

} // namespace os
