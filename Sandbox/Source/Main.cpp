#include <iostream>
#include <filesystem>

#include <Yuki/EntryPoint.hpp>
#include <Yuki/Core/Application.hpp>
#include <Yuki/Core/Logging.hpp>
#include <Yuki/EventSystem/ApplicationEvents.hpp>
#include <Yuki/Rendering/RHI/GraphicsPipelineBuilder.hpp>
#include <Yuki/Rendering/RHI/ShaderCompiler.hpp>
#include <Yuki/Rendering/RenderTarget.hpp>

class TestApplication : public Yuki::Application
{
public:
	TestApplication()
	    : Yuki::Application("Test Application")
	{
	}

private:
	Yuki::RenderTarget* m_RenderTarget;

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

		Yuki::RenderTargetInfo renderTargetInfo;
		renderTargetInfo.Width = 1280;
		renderTargetInfo.Height = 720;
		renderTargetInfo.ColorAttachments[0] = Yuki::ImageFormat::BGRA8UNorm;
		renderTargetInfo.DepthAttachmentFormat = Yuki::ImageFormat::D24UNormS8UInt;
		m_RenderTarget = GetRenderContext()->CreateRenderTarget(renderTargetInfo);
	}

	void OnDestroy() override
	{
		GetRenderContext()->DestroyRenderTarget(m_RenderTarget);
	}
};

YUKI_DECLARE_APPLICATION(TestApplication)
