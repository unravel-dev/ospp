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

/// A folder for files the application can rebuild, which the user or the system may delete, as an
/// absolute path ending in a separator: the application's cache folder on Android, where the names
/// are not used, and a "cache" folder inside the preferences folder elsewhere. The folder is created
/// when missing. Empty when the backend has no such folder or it cannot be created.
auto get_cache_path(const std::string& organization, const std::string& application) -> std::string;
} // namespace paths
} // namespace os
