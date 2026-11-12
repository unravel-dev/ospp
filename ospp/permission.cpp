#include "permission.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/permission.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(GLFW_BACKEND)
#include "impl/glfw/permission.hpp"
#define impl detail::glfw
#elif defined(MML_BACKEND)
#include "impl/mml/permission.hpp"
#define impl detail::mml
#else
#error "unsupported backend"
#endif

namespace os
{
namespace permission
{
auto is_requested_at_run_time() -> bool
{
	return impl::is_requested_at_run_time();
}

auto request(const std::string& name, result_callback on_result) -> bool
{
	return impl::request(name, std::move(on_result));
}
} // namespace permission
} // namespace os
