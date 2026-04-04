#ifndef MML_VIDEOMODEIMPL_HPP
#define MML_VIDEOMODEIMPL_HPP

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <mml/window/video_mode.hpp>


namespace mml
{
namespace priv
{
////////////////////////////////////////////////////////////
/// \brief OS-specific implementation of video modes functions
///
////////////////////////////////////////////////////////////
class video_mode_impl
{
public:
    static int get_number_of_displays();
    static std::vector<video_mode> get_desktop_modes(int index = 0);
    static video_mode get_desktop_mode(int index = 0);
    static video_bounds get_display_bounds(int index = 0);
	static video_bounds get_display_usable_bounds(int index = 0);
    /// GLFW-style scale for the display (effective DPI / 96 on Win32). 1 if unknown.
    static float get_display_content_scale(int index = 0);

};

} // namespace priv

} // namespace mml


#endif // MML_VIDEOMODEIMPL_HPP
