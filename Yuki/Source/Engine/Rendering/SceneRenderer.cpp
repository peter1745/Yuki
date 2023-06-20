#include "Rendering/SceneRenderer.hpp"
#include "Rendering/RHI/ShaderCompiler.hpp"

namespace Yuki {

	SceneRenderer::SceneRenderer(RenderContext* InContext, Viewport* InViewport)
		: m_Context(InContext), m_Viewport(InViewport)
	{
		m_RenderInterface = InContext->CreateRenderInterface();
		m_CommandPool = InContext->CreateCommandBufferPool({ .IsTransient = false });

		BuildPipelines();
	}

	void SceneRenderer::BeginDraw()
	{
		m_CommandPool->Reset();
		m_CommandBuffer = m_CommandPool->NewCommandBuffer();

		m_CommandBuffer->Begin();
		
		m_CommandBuffer->SetViewport(m_Viewport);

		m_CommandBuffer->BindPipeline(m_MeshPipeline.GetPtr());
		m_CommandBuffer->BeginRendering(m_Viewport);
	}

	void SceneRenderer::DrawMesh(const Mesh& InMesh)
	{
		m_CommandBuffer->BindVertexBuffer(InMesh.VertexBuffer);
		m_CommandBuffer->BindIndexBuffer(InMesh.IndexBuffer);
		m_CommandBuffer->DrawIndexed(InMesh.IndexCount, 1, 0, 0, 0);
	}

	void SceneRenderer::EndDraw()
	{
		m_CommandBuffer->EndRendering();
		m_CommandBuffer->End();
	}

	void SceneRenderer::BuildPipelines()
	{
		auto* pipelineBuilder = m_Context->CreateGraphicsPipelineBuilder();

		{
			auto shader = m_Context->GetShaderCompiler()->CompileFromFile("Resources/Shaders/Geometry.glsl");
			m_MeshPipeline = pipelineBuilder->WithShader(shader)
				->ColorAttachment(ImageFormat::BGRA8UNorm)
				->DepthAttachment()
				->AddVertexInput(0, ShaderDataType::Float3)
				->AddVertexInput(1, ShaderDataType::Float3)
				->AddVertexInput(2, ShaderDataType::Float2)
				->Build();
		}

		m_Context->DestroyGraphicsPipelineBuilder(pipelineBuilder);
	}

}
