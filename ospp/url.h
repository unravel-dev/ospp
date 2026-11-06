#pragma once

#include <string>

namespace os
{
namespace url
{
/// Opens \a address in the program the system opens such addresses with: a web page in the default browser.
/// Returns false when the backend cannot open it.
auto open(const std::string& address) noexcept -> bool;
} // namespace url
} // namespace os
