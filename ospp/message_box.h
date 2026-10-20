#pragma once

#include <cstdint>
#include <string>

namespace os
{
namespace message_box
{
enum class kind : uint8_t
{
	error,
	warning,
	information
};

/// Shows a message box with an OK button and waits until the user closes it. Returns false when the
/// backend cannot show one.
auto show(kind type, const std::string& title, const std::string& message) noexcept -> bool;
} // namespace message_box
} // namespace os
