#pragma once

#include <string>

namespace os
{
namespace paths
{
namespace detail
{
namespace glfw
{
// GLFW has no notion of a preferences folder.
inline auto get_preferences_path(const std::string&, const std::string&) -> std::string
{
	return {};
}
} // namespace glfw
} // namespace detail
} // namespace paths
} // namespace os
