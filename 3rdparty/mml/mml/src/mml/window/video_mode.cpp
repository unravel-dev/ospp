////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <mml/window/video_mode.hpp>
#include <mml/window/video_mode_impl.hpp>
#include <algorithm>
#include <functional>


namespace mml
{
////////////////////////////////////////////////////////////
int get_number_of_displays()
{
    return priv::video_mode_impl::get_number_of_displays();
}


////////////////////////////////////////////////////////////
video_mode video_mode::get_desktop_mode(int index)
{
    // Directly forward to the OS-specific implementation
    return priv::video_mode_impl::get_desktop_mode(index);
}


////////////////////////////////////////////////////////////
std::vector<video_mode> video_mode::get_desktop_modes(int index)
{
    auto modes = priv::video_mode_impl::get_desktop_modes(index);
    std::sort(modes.begin(), modes.end(), std::greater<video_mode>());

    return modes;
}

////////////////////////////////////////////////////////////
bool operator ==(const video_mode& left, const video_mode& right)
{    
    return std::tie(left.width, left.height, left.refresh_rate, left.bits_per_pixel) ==
           std::tie(right.width, right.height, right.refresh_rate, right.bits_per_pixel);
}


////////////////////////////////////////////////////////////
bool operator !=(const video_mode& left, const video_mode& right)
{
    return !(left == right);
}


////////////////////////////////////////////////////////////
bool operator <(const video_mode& left, const video_mode& right)
{
    return std::tie(left.width, left.height, left.refresh_rate, left.bits_per_pixel) <
           std::tie(right.width, right.height, right.refresh_rate, right.bits_per_pixel);
}


////////////////////////////////////////////////////////////
bool operator >(const video_mode& left, const video_mode& right)
{
    return right < left;
}


////////////////////////////////////////////////////////////
bool operator <=(const video_mode& left, const video_mode& right)
{
    return !(right < left);
}


////////////////////////////////////////////////////////////
bool operator >=(const video_mode& left, const video_mode& right)
{
    return !(left < right);
}

video_bounds video_bounds::get_display_bounds(int index)
{
    return priv::video_mode_impl::get_display_bounds(index);
}

video_bounds video_bounds::get_display_usable_bounds(int index)
{
	return priv::video_mode_impl::get_display_usable_bounds(index);
}

////////////////////////////////////////////////////////////
float get_display_content_scale(int display_index)
{
	return priv::video_mode_impl::get_display_content_scale(display_index);
}

} // namespace mml
