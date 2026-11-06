#include "touch.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/touch.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(MML_BACKEND)
#include "impl/mml/touch.hpp"
#define impl detail::mml
#elif defined(GLFW_BACKEND)
#include "impl/glfw/touch.hpp"
#define impl detail::glfw
#else
#error "unsupported backend"
#endif

namespace os
{
namespace touch
{
auto has_screen() noexcept -> bool
{
	return impl::has_screen();
}
} // namespace touch
} // namespace os
