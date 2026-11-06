#pragma once
#include "../../url.h"
#include "config.hpp"

namespace os
{
namespace url
{
namespace detail
{
namespace sdl
{
inline auto open(const std::string& address) noexcept -> bool
{
	return SDL_OpenURL(address.c_str());
}
} // namespace sdl
} // namespace detail
} // namespace url
} // namespace os
