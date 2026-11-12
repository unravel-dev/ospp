#pragma once
#include "../../permission.h"

namespace os
{
namespace permission
{
namespace detail
{
namespace mml
{
inline auto is_requested_at_run_time() -> bool
{
	return false;
}

inline auto request(const std::string& /*name*/, result_callback on_result) -> bool
{
	on_result(true);
	return true;
}
} // namespace mml
} // namespace detail
} // namespace permission
} // namespace os
