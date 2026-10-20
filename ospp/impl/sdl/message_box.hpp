#pragma once

#include "../../message_box.h"

#include "config.hpp"

namespace os
{
namespace message_box
{
namespace detail
{
namespace sdl
{
inline auto to_flags(kind type) noexcept -> SDL_MessageBoxFlags
{
	switch(type)
	{
		case kind::error:
			return SDL_MESSAGEBOX_ERROR;
		case kind::warning:
			return SDL_MESSAGEBOX_WARNING;
		case kind::information:
		default:
			return SDL_MESSAGEBOX_INFORMATION;
	}
}

inline auto show(kind type, const std::string& title, const std::string& message) noexcept -> bool
{
	return SDL_ShowSimpleMessageBox(to_flags(type), title.c_str(), message.c_str(), nullptr);
}
} // namespace sdl
} // namespace detail
} // namespace message_box
} // namespace os
