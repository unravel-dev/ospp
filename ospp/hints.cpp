#include "hints.h"

#if defined(SDL_BACKEND)
#include "impl/sdl/hints.hpp"
#ifndef impl
#define impl detail::sdl
#endif
#elif defined(MML_BACKEND)
#include "impl/mml/hints.hpp"
#define impl detail::mml
#elif defined(GLFW_BACKEND)
#include "impl/glfw/hints.hpp"
#define impl detail::glfw
#else
#error "unsupported backend"
#endif

namespace os
{
void set_hint(const std::string& hint, const std::string& value)
{
    impl::set_hint(hint, value);
}

} // namespace os
