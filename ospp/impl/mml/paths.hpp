#pragma once

#include <string>

namespace os
{
namespace paths
{
namespace detail
{
namespace mml
{
// MML has no notion of a preferences folder.
inline auto get_preferences_path(const std::string&, const std::string&) -> std::string
{
	return {};
}

// Nor of a cache folder.
inline auto get_cache_path(const std::string&, const std::string&) -> std::string
{
	return {};
}
} // namespace mml
} // namespace detail
} // namespace paths
} // namespace os
