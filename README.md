![windows](https://github.com/unravel-dev/ospp/actions/workflows/windows.yml/badge.svg)
![linux](https://github.com/unravel-dev/ospp/actions/workflows/linux.yml/badge.svg)
![macos](https://github.com/unravel-dev/ospp/actions/workflows/macos.yml/badge.svg)

## ospp c++14 cross-platform window and input library
- The library is JUST windowing and input.
- Wrapping the most popular windowing libraries under one api
- Supported backends - SDL, GLFW

```c++
int main()
{
	if(!os::init())
	{
		return -1;
	}

	os::window window{"win", os::window::centered, os::window::centered, 500, 500, os::window::resizable};
					  
	bool running = true;

        while(running)
	{
		os::event e{};
		while(os::poll_event(e))
		{
			if(e.type == os::events::quit)
			{
				std::cout << "quit (all windows were closed)" << std::endl;
				running = false;
				break;
			}

			if(e.type == os::events::key_down)
			{
				if(e.key.code == os::key::q)
				{
					std::cout << "quit ('q' key pressed)" << std::endl;
					running = false;
					break;
				}
			}
		}

		std::this_thread::sleep_for(16ms);
	}
	
    	return os::shutdown();
}
```
