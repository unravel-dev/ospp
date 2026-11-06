// The browser entry, built only for the browser. It lives in the library rather than in main.h because the host
// runtime that starts the page, such as the .NET browser runtime, never calls main: the host calls this function
// by the library's name (DllImport("libospp")) with the arguments it was given, as SDL's activity calls SDL_main on
// Android.
auto os_main(int argc, char* argv[]) -> int;

extern "C" __attribute__((used, visibility("default"))) int os_main_from_host(int argc, char* argv[])
{
	return os_main(argc, argv);
}
