#pragma once
#include "../../url.h"

namespace os
{
namespace url
{
namespace detail
{
namespace mml
{
inline auto open(const std::string& /*address*/) noexcept -> bool
{
	return false;
}
} // namespace mml
} // namespace detail
} // namespace url
} // namespace os
