#include "Rendering/SceneRenderer.hpp"
#include "Rendering/RHI/ShaderCompiler.hpp"
#include "Math/Math.hpp"

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

		m_FrameTransforms.ViewProjection = Math::Mat4::PerspectiveInfReversedZ(Math::Radians(90.0f), 1920.0f / 1080.0f, 0.05f);
		//m_FrameTransforms.ViewProjection *= Math::Mat4::Rotation(Math::Quat(Math::Radians(180.0f), { 0.0f, 1.0f, 0.0f }));
	}

	void SceneRenderer::DrawMesh(const LoadedMesh& InMesh)
	{
		for (const auto& meshInstance : InMesh.Instances)
		{
			m_FrameTransforms.Transform = meshInstance.Transform * Math::Mat4::Translation({ 0.0f, 0.0f, -2.0f });
			m_CommandBuffer->PushConstants(m_MeshPipeline.GetPtr(), &m_FrameTransforms, sizeof(FrameTransforms), 0);
			m_CommandBuffer->BindVertexBuffer(meshInstance.SourceMesh->VertexBuffer);
			m_CommandBuffer->BindIndexBuffer(meshInstance.SourceMesh->IndexBuffer);
			m_CommandBuffer->DrawIndexed(meshInstance.SourceMesh->Indices.size(), 1, 0, 0, 0);
		}
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
				->PushConstant(0, sizeof(FrameTransforms))
				->Build();
		}

		m_Context->DestroyGraphicsPipelineBuilder(pipelineBuilder);
	}

}
