////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <mml/window/video_mode_impl.hpp>
#include <windows.h>
#include <algorithm>


namespace mml
{
namespace priv
{

struct monitor_data
{
    WCHAR adapterName[CCHDEVICENAME] {};
    HMONITOR handle{};

	MONITORINFOEX mi;
};

BOOL WINAPI callback(HMONITOR handle,
        HDC /*dc*/,
        LPRECT /*rect*/,
        LPARAM data)
{
	monitor_data* monitor = (monitor_data*) data;
	ZeroMemory(&monitor->mi, sizeof(monitor->mi));
	monitor->mi.cbSize = sizeof(monitor->mi);

	if (GetMonitorInfo(handle, (MONITORINFO*) &monitor->mi))
    {
		if (wcscmp(monitor->mi.szDevice, monitor->adapterName) == 0)
            monitor->handle = handle;
    }

    return TRUE;
}

video_bounds get_display_bounds_impl(int index, bool usable)
{
	video_bounds bounds{};

	int disp_count{};

	DISPLAY_DEVICE adapter;
	DWORD adapter_index = 0;

	for(;;adapter_index++)
	{
		ZeroMemory(&adapter, sizeof(adapter));
		adapter.cb = sizeof(adapter);

		if (!EnumDisplayDevices(nullptr, adapter_index, &adapter, 0))
			break;

		if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
			continue;

		DWORD display_index = 0;
		for(;;display_index++)
		{
			DISPLAY_DEVICE display;
			ZeroMemory(&display, sizeof(display));
			display.cb = sizeof(display);

			if (!EnumDisplayDevices(adapter.DeviceName, display_index, &display, 0))
				break;

			if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
				continue;

			if(index == disp_count)
			{
				DEVMODE win32Mode;
				win32Mode.dmSize = sizeof(win32Mode);
				win32Mode.dmDriverExtra = 0;
				EnumDisplaySettings(adapter.DeviceName, ENUM_CURRENT_SETTINGS, &win32Mode);

				RECT rect;
				rect.left   = win32Mode.dmPosition.x;
				rect.top    = win32Mode.dmPosition.y;
				rect.right  = win32Mode.dmPosition.x + win32Mode.dmPelsWidth;
				rect.bottom = win32Mode.dmPosition.y + win32Mode.dmPelsHeight;

				monitor_data monitor;

				wcscpy_s(monitor.adapterName, adapter.DeviceName);
				EnumDisplayMonitors(nullptr, &rect, callback, (LPARAM)&monitor);


				auto usedRect = usable ? monitor.mi.rcWork : monitor.mi.rcMonitor;

				bounds.x = usedRect.left;
				bounds.y = usedRect.top;
				bounds.width = usedRect.right - usedRect.left;
				bounds.height = usedRect.bottom - usedRect.top;

				break;
			}

			disp_count++;
		}

		if(bounds.width != 0 && bounds.height != 0)
		{
			break;
		}
	}

	return bounds;
}

namespace
{
HMONITOR get_hmonitor_for_display_index(int index)
{
	int disp_count = 0;
	DISPLAY_DEVICE adapter;
	for(DWORD adapter_index = 0;; adapter_index++)
	{
		ZeroMemory(&adapter, sizeof(adapter));
		adapter.cb = sizeof(adapter);
		if(!EnumDisplayDevices(nullptr, adapter_index, &adapter, 0))
			break;
		if(!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
			continue;
		DWORD display_index = 0;
		for(;; display_index++)
		{
			DISPLAY_DEVICE display;
			ZeroMemory(&display, sizeof(display));
			display.cb = sizeof(display);
			if(!EnumDisplayDevices(adapter.DeviceName, display_index, &display, 0))
				break;
			if(!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
				continue;
			if(index == disp_count)
			{
				DEVMODE win32Mode;
				win32Mode.dmSize = sizeof(win32Mode);
				win32Mode.dmDriverExtra = 0;
				if(!EnumDisplaySettings(adapter.DeviceName, ENUM_CURRENT_SETTINGS, &win32Mode))
				{
					return nullptr;
				}
				RECT rect;
				rect.left = win32Mode.dmPosition.x;
				rect.top = win32Mode.dmPosition.y;
				rect.right = win32Mode.dmPosition.x + win32Mode.dmPelsWidth;
				rect.bottom = win32Mode.dmPosition.y + win32Mode.dmPelsHeight;
				monitor_data monitor;
				wcscpy_s(monitor.adapterName, adapter.DeviceName);
				EnumDisplayMonitors(nullptr, &rect, callback, (LPARAM)&monitor);
				HMONITOR hm = monitor.handle;
				if(!hm)
				{
					POINT pt;
					pt.x = (rect.left + rect.right) / 2;
					pt.y = (rect.top + rect.bottom) / 2;
					hm = MonitorFromPoint(pt, MONITOR_DEFAULTTOPRIMARY);
				}
				return hm;
			}
			disp_count++;
		}
	}
	return nullptr;
}
} // namespace

int video_mode_impl::get_number_of_displays()
{
    int disp_count{};

    DISPLAY_DEVICE adapter;
    DWORD adapter_index = 0;
    for(;;adapter_index++)
    {
        ZeroMemory(&adapter, sizeof(adapter));
        adapter.cb = sizeof(adapter);

        if (!EnumDisplayDevices(nullptr, adapter_index, &adapter, 0))
            break;

        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        DWORD display_index = 0;
        for(;;display_index++)
        {
            DISPLAY_DEVICE display;
            ZeroMemory(&display, sizeof(display));
            display.cb = sizeof(display);

            if (!EnumDisplayDevices(adapter.DeviceName, display_index, &display, 0))
                break;

            if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
                continue;

            disp_count++;
        }

        // HACK: If an active adapter does not have any display devices
        //       (as sometimes happens), add it directly as a monitor
        if (display_index == 0)
        {
            disp_count++;
        }

    }

    return disp_count;
}

////////////////////////////////////////////////////////////
std::vector<video_mode> video_mode_impl::get_desktop_modes(int index)
{
    std::vector<video_mode> modes;

    int disp_count{};

    DISPLAY_DEVICE adapter;
    DWORD adapter_index = 0;

    for(;;adapter_index++)
    {
        ZeroMemory(&adapter, sizeof(adapter));
        adapter.cb = sizeof(adapter);

        if (!EnumDisplayDevices(nullptr, adapter_index, &adapter, 0))
            break;

        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        DWORD display_index = 0;
        for(;;display_index++)
        {
            DISPLAY_DEVICE display;
            ZeroMemory(&display, sizeof(display));
            display.cb = sizeof(display);

            if (!EnumDisplayDevices(adapter.DeviceName, display_index, &display, 0))
                break;

            if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
                continue;

            if(index == disp_count)
            {
                // Enumerate all available video modes for the primary display adapter
                DEVMODE win32Mode;
                win32Mode.dmSize = sizeof(win32Mode);
                win32Mode.dmDriverExtra = 0;
                for (int count = 0; EnumDisplaySettings(nullptr, count, &win32Mode); ++count)
                {
                    // Convert to mml::video_mode
                    video_mode mode{};
                    mode.width = win32Mode.dmPelsWidth;
                    mode.height = win32Mode.dmPelsHeight;
                    mode.bits_per_pixel = win32Mode.dmBitsPerPel;
                    mode.refresh_rate = win32Mode.dmDisplayFrequency;

                    // Add it only if it is not already in the array
                    if (std::find(modes.begin(), modes.end(), mode) == modes.end())
                        modes.push_back(mode);
                }

                break;
            }

            disp_count++;
        }

        if(!modes.empty())
        {
            break;
        }
    }

    return modes;
}


////////////////////////////////////////////////////////////
video_mode video_mode_impl::get_desktop_mode(int index)
{
    video_mode mode{};

    int disp_count{};

    DISPLAY_DEVICE adapter;
    DWORD adapter_index = 0;

    for(;;adapter_index++)
    {
        ZeroMemory(&adapter, sizeof(adapter));
        adapter.cb = sizeof(adapter);

        if (!EnumDisplayDevices(nullptr, adapter_index, &adapter, 0))
            break;

        if (!(adapter.StateFlags & DISPLAY_DEVICE_ACTIVE))
            continue;

        DWORD display_index = 0;
        for(;;display_index++)
        {
            DISPLAY_DEVICE display;
            ZeroMemory(&display, sizeof(display));
            display.cb = sizeof(display);

            if (!EnumDisplayDevices(adapter.DeviceName, display_index, &display, 0))
                break;

            if (!(display.StateFlags & DISPLAY_DEVICE_ACTIVE))
                continue;

            if(index == disp_count)
            {
                DEVMODE win32Mode;
                win32Mode.dmSize = sizeof(win32Mode);
                win32Mode.dmDriverExtra = 0;
                EnumDisplaySettings(adapter.DeviceName, ENUM_CURRENT_SETTINGS, &win32Mode);


				RECT rect;
				rect.left   = win32Mode.dmPosition.x;
				rect.top    = win32Mode.dmPosition.y;
				rect.right  = win32Mode.dmPosition.x + win32Mode.dmPelsWidth;
				rect.bottom = win32Mode.dmPosition.y + win32Mode.dmPelsHeight;

				monitor_data monitor;

				wcscpy_s(monitor.adapterName, adapter.DeviceName);
				EnumDisplayMonitors(nullptr, &rect, callback, (LPARAM)&monitor);


				int monitor_height = monitor.mi.rcMonitor.bottom - monitor.mi.rcMonitor.top;
				int work_height = monitor.mi.rcWork.bottom - monitor.mi.rcWork.top;

				mode.display_scale = monitor_height / work_height;
				mode.width = monitor.mi.rcMonitor.right - monitor.mi.rcMonitor.left;
				mode.height = monitor.mi.rcMonitor.bottom - monitor.mi.rcMonitor.top;
                mode.bits_per_pixel = win32Mode.dmBitsPerPel;
                mode.refresh_rate = win32Mode.dmDisplayFrequency;

                break;
            }

            disp_count++;
        }

        if(mode.width != 0 && mode.height != 0)
        {
            break;
        }
    }

    return mode;
}

video_bounds video_mode_impl::get_display_bounds(int index)
{
	return get_display_bounds_impl(index, false);
}

video_bounds video_mode_impl::get_display_usable_bounds(int index)
{
	return get_display_bounds_impl(index, true);
}

////////////////////////////////////////////////////////////
float video_mode_impl::get_display_content_scale(int index)
{
	HMONITOR mon = get_hmonitor_for_display_index(index);
	if(!mon)
	{
		return 1.f;
	}
	using GetDpiForMonitorFn = HRESULT(WINAPI*)(HMONITOR, int, UINT*, UINT*);
	static auto get_dpi_for_monitor = reinterpret_cast<GetDpiForMonitorFn>(
		GetProcAddress(GetModuleHandleW(L"Shcore.dll"), "GetDpiForMonitor"));
	if(!get_dpi_for_monitor)
	{
		return 1.f;
	}
	UINT dpi_x = 0;
	UINT dpi_y = 0;
	// MDT_EFFECTIVE_DPI == 0 (shellscalingapi.h)
	const HRESULT hr = get_dpi_for_monitor(mon, 0, &dpi_x, &dpi_y);
	if(FAILED(hr) || dpi_x == 0 || dpi_y == 0)
	{
		return 1.f;
	}
	const UINT dpi = (std::max)(dpi_x, dpi_y);
	return static_cast<float>(dpi) / 96.f;
}

} // namespace priv

} // namespace mml
