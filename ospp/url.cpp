#include "url.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/url.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(GLFW_BACKEND)
#include "impl/glfw/url.hpp"
#define impl detail::glfw
#elif defined(MML_BACKEND)
#include "impl/mml/url.hpp"
#define impl detail::mml
#else
#error "unsupported backend"
#endif

namespace os
{
namespace url
{
auto open(const std::string& address) noexcept -> bool
{
	return impl::open(address);
}
} // namespace url
} // namespace os
