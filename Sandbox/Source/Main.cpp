#include <iostream>
#include <filesystem>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/RHI/GraphicsPipelineBuilder.hpp>
#include <Yuki/Rendering/RHI/ShaderCompiler.hpp>
#include <Yuki/Rendering/RHI/RenderTarget.hpp>

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
	}

	void OnRunLoop() override
	{
	}

	void OnDestroy() override
	{
	}
};

YUKI_DECLARE_APPLICATION(TestApplication)
