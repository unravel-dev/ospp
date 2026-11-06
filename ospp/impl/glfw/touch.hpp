#pragma once
#include "../../touch.h"

namespace os
{
namespace touch
{
namespace detail
{
namespace glfw
{
inline auto has_screen() noexcept -> bool
{
	return false;
}
} // namespace glfw
} // namespace detail
} // namespace touch
} // namespace os
