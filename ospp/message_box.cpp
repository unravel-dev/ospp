#include "message_box.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/message_box.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(GLFW_BACKEND)
#include "impl/glfw/message_box.hpp"
#define impl detail::glfw
#elif defined(MML_BACKEND)
#include "impl/mml/message_box.hpp"
#define impl detail::mml
#else
#error "unsupported backend"
#endif

namespace os
{
namespace message_box
{
auto show(kind type, const std::string& title, const std::string& message) noexcept -> bool
{
	return impl::show(type, title, message);
}
} // namespace message_box
} // namespace os
