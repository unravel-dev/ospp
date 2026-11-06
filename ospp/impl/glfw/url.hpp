#pragma once
#include "../../url.h"

namespace os
{
namespace url
{
namespace detail
{
namespace glfw
{
inline auto open(const std::string& /*address*/) noexcept -> bool
{
	return false;
}
} // namespace glfw
} // namespace detail
} // namespace url
} // namespace os
