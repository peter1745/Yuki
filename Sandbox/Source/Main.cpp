#include <iostream>

#include <Yuki/Core/GenericWindow.hpp>
#include <Yuki/Core/Logging.hpp>

int main()
{
	Yuki::LogInit();

	Yuki::WindowAttributes windowAttributes =
	{
		.Title = "Sandbox - Yuki",
		.Width = 1920,
		.Height = 1080
	};
	auto window = Yuki::GenericWindow::New(windowAttributes);
	window->Create();

	while (true)
	{
		window->ProcessEvents();
	}

	std::cin.get();
	return 0;
}
