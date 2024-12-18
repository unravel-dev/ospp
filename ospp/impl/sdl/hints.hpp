#pragma once

#include "error.hpp"
#include <cstring>
#include <map>

namespace os
{
namespace detail
{
namespace sdl
{
inline void set_hint(const std::string& hint, const std::string& value)
{
    static const std::map<std::string, std::string> hint_mapping =
    {
		{"HINT_WINDOW_ACTIVATE_WHEN_SHOWN", SDL_HINT_WINDOW_ACTIVATE_WHEN_SHOWN},
        {"HINT_MOUSE_FOCUS_CLICKTHROUGH", SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH},
        {"HINT_MOUSE_AUTO_CAPTURE", SDL_HINT_MOUSE_AUTO_CAPTURE},
        {"HINT_BORDERLESS_WINDOWED_STYLE", "SDL_BORDERLESS_WINDOWED_STYLE"},
    };

    auto it = hint_mapping.find(hint);
    if(it != hint_mapping.end())
    {
        const auto& sdl_hint = it->second;
        SDL_SetHint(sdl_hint.c_str(), value.c_str());

    }

}
} // namespace sdl
} // namespace detail
} // namespace os
