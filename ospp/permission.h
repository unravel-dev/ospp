#pragma once

#include <functional>
#include <string>

namespace os
{
namespace permission
{
/// Gets the user's answer to a request. It may run on a thread of the system's, and inside request().
using result_callback = std::function<void(bool is_granted)>;

/// Whether the user grants permissions while the application runs (Android). Elsewhere a request is granted at
/// once: the system has no such permissions, or asks by itself when a device is first used (a browser).
auto is_requested_at_run_time() -> bool;

/// Asks the user for the permission \a name, as the platform names it ("android.permission.CAMERA"). The system
/// shows its dialog unless the user answered that permission before, and denies one the application's manifest
/// does not declare. Returns false, without calling \a on_result, when the request could not be made.
auto request(const std::string& name, result_callback on_result) -> bool;
} // namespace permission
} // namespace os
