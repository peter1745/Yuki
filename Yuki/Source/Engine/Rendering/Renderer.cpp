#include "Rendering/Renderer.hpp"
#include "Rendering/RHI/ShaderCompiler.hpp"
#include "Rendering/RHI/CommandBuffer.hpp"
#include "Rendering/RHI/Queue.hpp"
#include "../Rendering/RHI/Vulkan/VulkanBuffer.hpp"

namespace Yuki {

	struct Vertex { float Position[3]; };

	Buffer* vertexBuffer;

	Renderer::Renderer(RenderContext* InContext)
		: m_Context(InContext)
	{
		m_RenderInterface = InContext->CreateRenderInterface();
		m_CommandBuffer = InContext->CreateCommandBuffer();

		BufferInfo stagingBufferInfo =
		{
			.Type = BufferType::StagingBuffer,
			.Size = 10 * 1024 * 1024,
			.PersistentlyMapped = true,
		};
		m_StagingBuffer = InContext->CreateBuffer(stagingBufferInfo);
		
		BufferInfo vertexBufferInfo =
		{
			.Type = BufferType::VertexBuffer,
			.Size = sizeof(Vertex) * 3
		};
		vertexBuffer = InContext->CreateBuffer(vertexBufferInfo);

		Vertex vertices[3] =
		{
			{ { -0.5f,  0.5f, 0.0f } },
			{ {  0.5f,  0.5f, 0.0f } },
			{ {  0.0f, -0.5f, 0.0f } },
		};
		m_StagingBuffer->SetData(vertices, 3 * sizeof(Vertex));
		vertexBuffer->UploadData(m_StagingBuffer);

		BuildPipelines();
	}

	Renderer::~Renderer()
	{
		m_Context->DestroyBuffer(vertexBuffer);
	}

	void Renderer::Begin()
	{
		m_CommandBuffer->Begin();
	}

	void Renderer::DrawTriangle()
	{
		m_CommandBuffer->BindPipeline(m_TrianglePipeline.GetPtr());
		m_CommandBuffer->BindVertexBuffer(vertexBuffer);
		m_CommandBuffer->Draw(3, 1, 0, 0);
	}

	void Renderer::End()
	{
		m_CommandBuffer->End();
		//m_Context->GetGraphicsQueue()->SubmitCommandBuffers({m_CommandBuffer}, {}, {});
	}

	void Renderer::BuildPipelines()
	{
		auto* pipelineBuilder = m_Context->CreateGraphicsPipelineBuilder();

		{
			auto triangleShader = m_Context->GetShaderCompiler()->CompileFromFile("Resources/Shaders/Test.glsl");
			m_TrianglePipeline = pipelineBuilder->WithShader(triangleShader)
									 ->ColorAttachment(ImageFormat::BGRA8UNorm)
									 ->AddVertexInput(0, ShaderDataType::Float3)
									 ->Build();
		}

		m_Context->DestroyGraphicsPipelineBuilder(pipelineBuilder);
	}

}
