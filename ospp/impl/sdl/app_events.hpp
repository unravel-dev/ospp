#pragma once

#include "../../event.h"
#include "config.hpp"

#include <utility>

namespace os
{
namespace detail
{
namespace sdl
{
inline auto get_app_event_callback() -> app_event_callback&
{
	static app_event_callback callback;
	return callback;
}

inline auto to_app_event(uint32_t type) -> events
{
	switch(type)
	{
		case SDL_EVENT_TERMINATING:
			return events::app_terminating;
		case SDL_EVENT_LOW_MEMORY:
			return events::app_low_memory;
		case SDL_EVENT_WILL_ENTER_BACKGROUND:
			return events::app_will_enter_background;
		case SDL_EVENT_DID_ENTER_BACKGROUND:
			return events::app_did_enter_background;
		case SDL_EVENT_WILL_ENTER_FOREGROUND:
			return events::app_will_enter_foreground;
		case SDL_EVENT_DID_ENTER_FOREGROUND:
			return events::app_did_enter_foreground;
		default:
			return events::unknown;
	}
}

/// SDL calls its event watches as it adds each event to its queue, on the adding thread. Mobile
/// platforms add the lifecycle events on the thread that pumps events, right before the pump may
/// block, so the callback runs before the application stops.
inline auto watch_app_events(void* /*userdata*/, SDL_Event* e) -> bool
{
	const events type = to_app_event(e->type);
	if(type != events::unknown)
	{
		const auto& callback = get_app_event_callback();
		if(callback)
		{
			callback(type);
		}
	}
	return true;
}

inline void set_app_event_callback(app_event_callback callback)
{
	get_app_event_callback() = std::move(callback);
}
} // namespace sdl
} // namespace detail
} // namespace os
