#include "App.hpp"

namespace Yuki {

	void Application::Run()
	{
		OnRun();

		while (m_Running)
		{
			OnUpdate();
		}
	}

}
