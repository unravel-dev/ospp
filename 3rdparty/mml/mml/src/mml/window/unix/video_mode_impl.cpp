////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <X11/Xlib.h>
#include <X11/extensions/Xrandr.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <mml/system/err.hpp>
#include <mml/window/unix/display.hpp>
#include <mml/window/video_mode_impl.hpp>

namespace mml
{
namespace priv
{
namespace
{
const XRRModeInfo* get_mode_info(const XRRScreenResources* sr, RRMode id)
{
	int i;

	for(i = 0; i < sr->nmode; i++)
	{
		if(sr->modes[i].id == id)
			return sr->modes + i;
	}

	return nullptr;
}

static int calculate_refresh_rate(const XRRModeInfo* mi)
{
	if(mi->hTotal && mi->vTotal)
		return int(round(double(mi->dotClock) / (double(mi->hTotal) * double(mi->vTotal))));
	else
		return 0;
}

// Check whether the display mode should be included in enumeration
//
static bool mode_is_good(const XRRModeInfo* mi)
{
	return (mi->modeFlags & RR_Interlace) == 0;
}

struct on_exit
{
	on_exit(std::function<void()> callback)
		: _(std::move(callback))
	{
	}
	~on_exit()
	{
		if(_)
			_();
	}

	std::function<void()> _{};
};

bool is_valid(Display* display, XRRScreenResources* resources, RROutput output)
{
	const auto output_info = XRRGetOutputInfo(display, resources, output);
	if(!output_info)
	{
		return false;
	}

	const on_exit _([&] { XRRFreeOutputInfo(output_info); });

	if(output_info->connection != RR_Connected)
	{
		return false;
	}

	const auto& crtc_id = output_info->crtc;
	if(crtc_id == 0)
	{
		return false;
	}

	const auto crtc_info = XRRGetCrtcInfo(display, resources, crtc_id);
	if(!crtc_info)
	{
		return false;
	}

	XRRFreeCrtcInfo(crtc_info);

	return true;
}
} // namespace
////////////////////////////////////////////////////////////
int video_mode_impl::get_number_of_displays()
{
	Display* display = open_display();
	if(!display)
	{
		return 0;
	}

	int version{};
	if(!XQueryExtension(display, "RANDR", &version, &version, &version))
	{
		close_display(display);
		return 0;
	}

	int connected_displays{};
	int screen = DefaultScreen(display);
	auto resources = XRRGetScreenResources(display, RootWindow(display, screen));
	if(resources)
	{
		for(auto i = 0; i < resources->noutput; i++)
		{
			auto& current_output = resources->outputs[i];
			if(is_valid(display, resources, current_output))
			{
				connected_displays++;
			}
		}

		XRRFreeScreenResources(resources);
	}

	close_display(display);

	return connected_displays;
}

std::vector<video_mode> video_mode_impl::get_desktop_modes(int index)
{
	Display* display = open_display();
	if(!display)
	{
		return {};
	}

	int version{};
	if(!XQueryExtension(display, "RANDR", &version, &version, &version))
	{
		close_display(display);
		return {};
	}

	int screen = DefaultScreen(display);
	auto resources = XRRGetScreenResources(display, RootWindow(display, screen));
	std::vector<video_mode> modes;

	if(resources)
	{
		int inner_index{};
		for(auto i = 0; i < resources->noutput; i++)
		{
			auto& current_output = resources->outputs[i];
			auto output_info = XRRGetOutputInfo(display, resources, current_output);
			if(output_info)
			{
				if(is_valid(display, resources, current_output))
				{
					if(inner_index == index)
					{
						auto& crtc_id = output_info->crtc;
						auto crtc_info = XRRGetCrtcInfo(display, resources, crtc_id);
						if(crtc_info)
						{
							for(auto idx = 0; idx < output_info->nmode; idx++)
							{
								auto& mode_id = output_info->modes[idx];
								auto mode_info = get_mode_info(resources, mode_id);
								if(mode_info && mode_is_good(mode_info))
								{
									modes.emplace_back();
									auto& mode = modes.back();
									if(crtc_info->rotation & (RR_Rotate_90 | RR_Rotate_270))
									{
										mode.width = mode_info->height;
										mode.height = mode_info->width;
									}
									else
									{
										mode.width = mode_info->width;
										mode.height = mode_info->height;
									}
									mode.bits_per_pixel = unsigned(DefaultDepth(display, screen));
									mode.refresh_rate = unsigned(calculate_refresh_rate(mode_info));
								}
							}
							XRRFreeCrtcInfo(crtc_info);
						}
					}

					inner_index++;
				}

				XRRFreeOutputInfo(output_info);

				if(!modes.empty())
				{
					break;
				}
			}
		}

		XRRFreeScreenResources(resources);
	}

	close_display(display);

	return modes;
}

////////////////////////////////////////////////////////////
video_mode video_mode_impl::get_desktop_mode(int index)
{
	Display* display = open_display();
	if(!display)
	{
		return {};
	}

	int version{};
	if(!XQueryExtension(display, "RANDR", &version, &version, &version))
	{
		close_display(display);
		return {};
	}

	int screen = DefaultScreen(display);
	auto resources = XRRGetScreenResources(display, RootWindow(display, screen));
	video_mode mode{};
	if(resources)
	{
		int inner_index{};
		for(auto i = 0; i < resources->noutput; i++)
		{
			auto& current_output = resources->outputs[i];
			auto output_info = XRRGetOutputInfo(display, resources, current_output);
			if(output_info)
			{
				if(is_valid(display, resources, current_output))
				{
					if(inner_index == index)
					{
						auto& crtc_id = output_info->crtc;
						auto crtc_info = XRRGetCrtcInfo(display, resources, crtc_id);
						if(crtc_info)
						{
							auto& current_mode = crtc_info->mode;
							auto mode_info = get_mode_info(resources, current_mode);
							if(mode_info)
							{
								if(crtc_info->rotation & (RR_Rotate_90 | RR_Rotate_270))
								{
									mode.width = mode_info->height;
									mode.height = mode_info->width;
								}
								else
								{
									mode.width = mode_info->width;
									mode.height = mode_info->height;
								}
								mode.bits_per_pixel = unsigned(DefaultDepth(display, screen));
								mode.refresh_rate = unsigned(calculate_refresh_rate(mode_info));
							}
							XRRFreeCrtcInfo(crtc_info);
						}
					}

					inner_index++;
				}

				XRRFreeOutputInfo(output_info);

				if(mode.width != 0 && mode.height != 0)
				{
					break;
				}
			}
		}

		XRRFreeScreenResources(resources);
	}

	close_display(display);

	return mode;
}

video_bounds video_mode_impl::get_display_bounds(int index)
{
	Display* display = open_display();
	if(!display)
	{
		return {};
	}

	int version{};
	if(!XQueryExtension(display, "RANDR", &version, &version, &version))
	{
		close_display(display);
		return {};
	}

	int screen = DefaultScreen(display);
	auto resources = XRRGetScreenResources(display, RootWindow(display, screen));
	video_bounds bounds{};
	if(resources)
	{
		int inner_index{};
		for(auto i = 0; i < resources->noutput; i++)
		{
			auto& current_output = resources->outputs[i];
			auto output_info = XRRGetOutputInfo(display, resources, current_output);
			if(output_info)
			{
				if(is_valid(display, resources, current_output))
				{
					if(inner_index == index)
					{
						auto& crtc_id = output_info->crtc;
						if(crtc_id != 0)
						{
							auto crtc_info = XRRGetCrtcInfo(display, resources, crtc_id);
							if(crtc_info)
							{
								bounds.x = crtc_info->x;
								bounds.y = crtc_info->y;

								auto& current_mode = crtc_info->mode;
								auto mode_info = get_mode_info(resources, current_mode);
								if(mode_info)
								{
									if(crtc_info->rotation & (RR_Rotate_90 | RR_Rotate_270))
									{
										bounds.width = mode_info->height;
										bounds.height = mode_info->width;
									}
									else
									{
										bounds.width = mode_info->width;
										bounds.height = mode_info->height;
									}
								}
								XRRFreeCrtcInfo(crtc_info);
							}
						}
					}

					inner_index++;
				}

				XRRFreeOutputInfo(output_info);

				if(bounds.width != 0 && bounds.height != 0)
				{
					break;
				}
			}
		}

		XRRFreeScreenResources(resources);
	}

	close_display(display);

	return bounds;
}

video_bounds video_mode_impl::get_display_usable_bounds(int index)
{
	return get_display_bounds(index);
}

////////////////////////////////////////////////////////////
float video_mode_impl::get_display_content_scale(int /*index*/)
{
	return 1.f;
}

} // namespace priv

} // namespace mml
