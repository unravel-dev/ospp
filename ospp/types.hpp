#pragma once
#include <cstdint>
#include <vector>

namespace os
{
template <typename T>
struct vec2d
{
	vec2d() = default;
	vec2d(T xval, T yval)
		: x(xval)
		, y(yval)
	{
	}

	T x{};
	T y{};
};

template <typename T>
struct area2d
{
	area2d() = default;
	area2d(T wval, T hval)
		: w(wval)
		, h(hval)
	{
	}

	T w{};
	T h{};
};

using point = vec2d<int32_t>;
using area = area2d<uint32_t>;

/// Pixel widths of the window frame outside the client area (OS decorations).
/// \a top includes the title bar. Values are non-negative when reported by the platform.
struct frame_insets
{
	int32_t left{};
	int32_t top{};
	int32_t right{};
	int32_t bottom{};
};

struct image
{
	std::vector<uint8_t> pixels;
	area size{};
};
} // namespace os
