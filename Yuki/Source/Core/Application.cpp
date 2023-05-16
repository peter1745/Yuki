#include "Core/Application.hpp"

namespace Yuki {

	Application::Application(const std::string& InName)
	    : m_Name(InName)
	{
	}

	void Application::Initialize()
	{
		WindowAttributes windowAttributes =
		{
			.Title = m_Name,
			.Width = 1920,
			.Height = 1080,
			.Maximized = true
		};

		m_Window = GenericWindow::New(windowAttributes);
		m_Window->Create();

		OnInitialize();
	}

	void Application::Run()
	{
		m_Window->Show();

		while (!m_Window->ShouldClose())
		{
			m_Window->ProcessEvents();

			OnRunLoop();
		}
	}

}
