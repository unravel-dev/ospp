#include "bundle.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/bundle.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(GLFW_BACKEND)
#include "impl/glfw/bundle.hpp"
#define impl detail::glfw
#elif defined(MML_BACKEND)
#include "impl/mml/bundle.hpp"
#define impl detail::mml
#else
#error "unsupported backend"
#endif

namespace os
{
namespace bundle
{
auto is_supported() -> bool
{
	return impl::is_supported();
}

auto open_region(const std::string& name) -> file_region
{
	return impl::open_region(name);
}

auto read_file(const std::string& name, std::string& content) -> bool
{
	return impl::read_file(name, content);
}
} // namespace bundle
} // namespace os
