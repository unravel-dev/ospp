#pragma once

/*
 * The application's entry point.
 *
 * Include this header in exactly one source file of an application and define os_main there. The
 * header defines the platform's own entry points, which all forward to os_main with the process
 * arguments:
 *  - main everywhere but Android;
 *  - wmain, WinMain and wWinMain on Windows (Unicode console and GUI subsystem builds);
 *  - SDL_main on Android, exported: SDL's Java activity loads the application's shared library
 *    and calls it by name on its own thread.
 */
auto os_main(int argc, char* argv[]) -> int;

#if defined(__ANDROID__)
extern "C" __attribute__((visibility("default"))) int SDL_main(int argc, char* argv[])
{
	return os_main(argc, argv);
}
#else
int main(int argc, char* argv[])
{
	return os_main(argc, argv);
}
#endif

#if defined(_WIN32)

// Declares WinMain and wWinMain without including windows.h.
#ifndef WINAPI
#define WINAPI __stdcall
#endif

typedef struct HINSTANCE__* HINSTANCE;
typedef char* LPSTR;
typedef wchar_t* PWSTR;

// Unicode console applications built with MSVC start in wmain.
#if defined(_MSC_VER) && defined(UNICODE)
extern "C" int wmain(int argc, wchar_t* wargv[], wchar_t* wenvp)
{
	(void)argc;
	(void)wargv;
	(void)wenvp;
	// The CRT keeps an ANSI copy of the arguments in __argc / __argv, also in Unicode builds.
	return os_main(__argc, __argv);
}
#endif

// GUI subsystem applications start in WinMain, or wWinMain in Unicode builds.
extern "C"
{
#if defined(UNICODE)
	int WINAPI wWinMain(HINSTANCE instance, HINSTANCE previous_instance, PWSTR command_line, int show_command)
#else
	int WINAPI WinMain(HINSTANCE instance, HINSTANCE previous_instance, LPSTR command_line, int show_command)
#endif
	{
		(void)instance;
		(void)previous_instance;
		(void)command_line;
		(void)show_command;
		return os_main(__argc, __argv);
	}
} // extern "C"

#endif // _WIN32
