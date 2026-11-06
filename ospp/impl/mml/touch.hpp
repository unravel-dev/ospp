#pragma once
#include "../../touch.h"

namespace os
{
namespace touch
{
namespace detail
{
namespace mml
{
inline auto has_screen() noexcept -> bool
{
	return false;
}
} // namespace mml
} // namespace detail
} // namespace touch
} // namespace os
