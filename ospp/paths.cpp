#include "paths.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/paths.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(GLFW_BACKEND)
#include "impl/glfw/paths.hpp"
#define impl detail::glfw
#elif defined(MML_BACKEND)
#include "impl/mml/paths.hpp"
#define impl detail::mml
#else
#error "unsupported backend"
#endif

namespace os
{
namespace paths
{
auto get_preferences_path(const std::string& organization, const std::string& application) -> std::string
{
	return impl::get_preferences_path(organization, application);
}
} // namespace paths
} // namespace os
