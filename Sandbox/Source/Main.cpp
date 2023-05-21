#include <iostream>
#include <filesystem>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>

class TestApplication : public Yuki::Application
{
public:
	TestApplication()
	    : Yuki::Application("Test Application")
	{
	}

private:
	void OnInitialize() override
	{
		Yuki::LogInfo("OnInitialize!");

		GetEventSystem().AddListener(this, &TestApplication::OnCloseEvent);

		Yuki::WindowAttributes windowAttributes = {
			.Title = "Second Window",
			.Width = 1280,
			.Height = 720
		};
		auto* otherWindow = NewWindow(windowAttributes);
		otherWindow->Show();

		GetRenderContext()->GetShaderCompiler()->CompileFromFile("Resources/Shaders/Test.glsl");
	}

	void OnRunLoop() override
	{
	}

private:
	void OnCloseEvent(const Yuki::ApplicationCloseEvent& InEvent)
	{
		Yuki::LogWarn("OnCloseEvent!");
	}
};

YUKI_DECLARE_APPLICATION(TestApplication)
