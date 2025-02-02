#pragma once

#include "../../display_mode.h"

#include "error.hpp"
#include <vector>
#include <algorithm>
#include <tuple>

namespace os
{
namespace detail
{
namespace glfw
{
namespace
{
inline auto from_impl(const GLFWvidmode& mode, float display_scale = 1.0f) -> ::os::display::mode
{
	::os::display::mode result;
	result.w = static_cast<uint32_t>(mode.width);
	result.h = static_cast<uint32_t>(mode.height);
	result.refresh_rate = static_cast<uint32_t>(mode.refreshRate);
	result.bpp = static_cast<uint32_t>(mode.redBits + mode.greenBits + mode.blueBits);
	result.display_scale = display_scale;

	result.w = static_cast<uint32_t>(float(result.w) / display_scale);
	result.h = static_cast<uint32_t>(float(result.h) / display_scale);

	return result;
}
} // namespace

inline auto number_of_video_displays() -> int
{
	int result{0};
	glfwGetMonitors(&result);
	if(result < 1)
	{
		OS_GLFW_ERROR_HANDLER(result);
	}
	return result;
}

inline auto get_primary_display_index() -> int
{
	return 0;
}


inline auto get_available_modes(int index = 0) -> std::vector<::os::display::mode>
{
	int mointor_count{0};
	auto monitors = glfwGetMonitors(&mointor_count);
	auto monitor = monitors[index];
	int count{0};
	auto modes = glfwGetVideoModes(monitor, &count);
	std::vector<::os::display::mode> result;
	if(count < 1)
	{
		OS_GLFW_ERROR_HANDLER(result);
	}

	for(int i = 0; i < count; ++i)
	{
		auto mode = modes[i];
		result.push_back(from_impl(mode));
	}

	std::sort(std::begin(result), std::end(result),
			  [](const auto& lhs, const auto& rhs)
			  {
				  return std::make_tuple(lhs.w, lhs.h, lhs.refresh_rate, lhs.bpp, lhs.display_scale) >
						 std::make_tuple(rhs.w, rhs.h, rhs.refresh_rate, rhs.bpp, rhs.display_scale);
			  });

	return result;
}

inline auto get_desktop_mode(int index = 0) -> ::os::display::mode
{
	int mointor_count{0};
	auto monitors = glfwGetMonitors(&mointor_count);
	auto monitor = monitors[index];
	auto result = glfwGetVideoMode(monitor);
	if(result)
	{
		float xscale = 1.0f;
		float yscale = 1.0f;
		glfwGetMonitorContentScale(monitor, &xscale, &yscale);
		auto mode = from_impl(*result, std::max(xscale, yscale));
		return mode;
	}

	return {};
}

inline auto get_display_bounds(int index = 0) -> ::os::display::bounds
{
	int mointor_count{0};
	auto monitors = glfwGetMonitors(&mointor_count);
	auto monitor = monitors[index];

	::os::display::bounds result{};
	int width{};
	int height{};
	glfwGetMonitorWorkarea(monitor, &result.x, &result.y, &width, &height);

#ifdef _WIN32
	height += 60;
#endif

	float xscale = 1.0f;
	float yscale = 1.0f;
	glfwGetMonitorContentScale(monitor, &xscale, &yscale);
	result.w = static_cast<uint32_t>(float(width) / xscale);
	result.h = static_cast<uint32_t>(float(height) / yscale);

	return result;
}

inline auto get_display_usable_bounds(int index = 0) -> ::os::display::bounds
{
	int mointor_count{0};
	auto monitors = glfwGetMonitors(&mointor_count);
	auto monitor = monitors[index];

	::os::display::bounds result{};
	int width{};
	int height{};
	glfwGetMonitorWorkarea(monitor, &result.x, &result.y, &width, &height);

	float xscale = 1.0f;
	float yscale = 1.0f;
	glfwGetMonitorContentScale(monitor, &xscale, &yscale);
	result.w = static_cast<uint32_t>(float(width) / xscale);
	result.h = static_cast<uint32_t>(float(height) / yscale);

	return result;
}

inline auto get_content_scale(int index = 0) -> float
{
	int mointor_count{0};
	auto monitors = glfwGetMonitors(&mointor_count);
	auto monitor = monitors[index];

	float xscale = 1.0f;
	float yscale = 1.0f;
	glfwGetMonitorContentScale(monitor, &xscale, &yscale);

	return std::max(xscale, yscale);
}

} // namespace glfw
} // namespace detail
} // namespace os
