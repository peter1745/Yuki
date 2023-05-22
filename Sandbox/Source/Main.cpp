#include <iostream>
#include <filesystem>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/GraphicsPipelineBuilder.hpp>

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

		Yuki::WindowAttributes windowAttributes = {
			.Title = "Second Window",
			.Width = 1280,
			.Height = 720
		};
		auto* otherWindow = NewWindow(windowAttributes);
		otherWindow->Show();

		auto testShader = GetRenderContext()->GetShaderCompiler()->CompileFromFile("Resources/Shaders/Test.glsl");

		auto pipelineBuilder = Yuki::GraphicsPipelineBuilder::New(GetRenderAPI(), GetRenderContext());
		auto testPipeline = pipelineBuilder->WithShader(testShader)
		                        ->ColorAttachment(Yuki::ImageFormat::BGRA8UNorm)
		                        ->DepthAttachment()
		                        ->Build();
	}

	void OnRunLoop() override
	{
	}
};

YUKI_DECLARE_APPLICATION(TestApplication)
