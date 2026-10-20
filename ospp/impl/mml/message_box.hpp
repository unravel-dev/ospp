#pragma once

#include "../../message_box.h"

namespace os
{
namespace message_box
{
namespace detail
{
namespace mml
{
/// MML has no message boxes.
inline auto show(kind /*type*/, const std::string& /*title*/, const std::string& /*message*/) noexcept -> bool
{
	return false;
}
} // namespace mml
} // namespace detail
} // namespace message_box
} // namespace os
