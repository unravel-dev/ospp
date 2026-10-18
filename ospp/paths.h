#pragma once

#include <string>

namespace os
{
namespace paths
{
/// A folder the application can always write to, as an absolute path ending in a separator: the
/// per-user preferences folder of `organization` / `application` on desktop, and the application's
/// internal storage on Android, where the names are not used. The folder is created when missing.
/// Empty when the backend has no such folder or it cannot be created.
auto get_preferences_path(const std::string& organization, const std::string& application) -> std::string;
} // namespace paths
} // namespace os
