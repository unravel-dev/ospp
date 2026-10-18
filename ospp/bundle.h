#pragma once

#include <cstdint>
#include <string>

namespace os
{
namespace bundle
{
/// A file packaged with the application as a byte range of a file that can be memory-mapped. The
/// descriptor is the caller's to close; it is -1 when the file is missing or stored compressed.
struct file_region
{
	int descriptor{-1};
	std::uint64_t offset{};
	std::uint64_t size{};
};

/// Whether the application's files are packaged with it rather than lying in folders next to it:
/// true on Android, where they are the APK's assets.
auto is_supported() -> bool;

/// The region of a packaged file stored uncompressed. `name` is '/' separated, relative to the
/// package root ("data/engine/content.pak").
auto open_region(const std::string& name) -> file_region;

/// A packaged file's bytes; false when it is missing or cannot be read.
auto read_file(const std::string& name, std::string& content) -> bool;
} // namespace bundle
} // namespace os
