#pragma once

#include "../../bundle.h"

namespace os
{
namespace bundle
{
namespace detail
{
namespace glfw
{
// GLFW runs only where the application's files lie in folders.
inline auto is_supported() -> bool
{
	return false;
}

inline auto open_region(const std::string&) -> file_region
{
	return {};
}

inline auto read_file(const std::string&, std::string&) -> bool
{
	return false;
}
} // namespace glfw
} // namespace detail
} // namespace bundle
} // namespace os
