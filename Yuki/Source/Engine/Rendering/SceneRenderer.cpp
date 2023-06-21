#include "Rendering/SceneRenderer.hpp"
#include "Rendering/RHI/ShaderCompiler.hpp"
#include "Rendering/RHI/Queue.hpp"
#include "Rendering/RHI/GraphicsPipelineBuilder.hpp"
#include "Rendering/RHI/SetLayoutBuilder.hpp"
#include "Math/Math.hpp"

namespace Yuki {

	SceneRenderer::SceneRenderer(RenderContext* InContext, Viewport* InViewport)
		: m_Context(InContext), m_Viewport(InViewport)
	{
		m_RenderInterface = InContext->CreateRenderInterface();
		m_CommandPool = InContext->CreateCommandBufferPool({ .IsTransient = false });
		m_StagingBuffer = InContext->CreateBuffer(
		{
			.Type = BufferType::StagingBuffer,
			.Size = 100 * 1024 * 1024,
			.PersistentlyMapped = true,
		});

		CreateDescriptorSets();
		BuildPipelines();
	}

	void SceneRenderer::BeginDraw(const Math::Mat4& InViewMatrix)
	{
		m_CommandPool->Reset();
		m_CommandBuffer = m_CommandPool->NewCommandBuffer();

		m_CommandBuffer->Begin();
		
		m_CommandBuffer->SetViewport(m_Viewport);

		m_CommandBuffer->BindPipeline(m_MeshPipeline.GetPtr());
		m_CommandBuffer->BeginRendering(m_Viewport);

		m_FrameTransforms.ViewProjection = Math::Mat4::PerspectiveInfReversedZ(Math::Radians(70.0f), 1920.0f / 1080.0f, 0.05f) * Math::Mat4::InvertAffine(InViewMatrix);
	}

	void SceneRenderer::DrawMesh(LoadedMesh& InMesh)
	{
		m_CommandBuffer->BindDescriptorSets(m_MeshPipeline.GetPtr(), std::array{ m_MaterialDescriptorSet });

		if (InMesh.Textures.empty())
		{
			for (const auto& loadedImage : InMesh.LoadedImages)
			{
				CommandBuffer* commandBuffer = m_CommandPool->NewCommandBuffer();
				commandBuffer->Begin();

				Image2D* image = m_Context->CreateImage2D(loadedImage.Width, loadedImage.Height, ImageFormat::RGBA8UNorm);

				m_StagingBuffer->SetData(loadedImage.Data.data(), uint32_t(loadedImage.Data.size()));
				commandBuffer->CopyToImage(image, m_StagingBuffer, 0);
				commandBuffer->End();
				
				m_Context->GetGraphicsQueue()->SubmitCommandBuffers({ commandBuffer }, {}, {});
				m_Context->GetGraphicsQueue()->WaitIdle();

				InMesh.Textures.emplace_back(std::move(image));
			}
		}

		m_MaterialDescriptorSet->Write(0, InMesh.Textures, m_Sampler);

		for (auto& meshInstance : InMesh.Instances)
		{
			m_FrameTransforms.Transform = meshInstance.Transform;
			m_CommandBuffer->PushConstants(m_MeshPipeline.GetPtr(), &m_FrameTransforms, sizeof(FrameTransforms), 0);
			m_CommandBuffer->BindVertexBuffer(meshInstance.SourceMesh->VertexBuffer);
			m_CommandBuffer->BindIndexBuffer(meshInstance.SourceMesh->IndexBuffer);
			m_CommandBuffer->DrawIndexed(uint32_t(meshInstance.SourceMesh->Indices.size()), 1, 0, 0, 0);
		}
	}

	void SceneRenderer::EndDraw()
	{
		m_CommandBuffer->EndRendering();
		m_CommandBuffer->End();
	}

	void SceneRenderer::CreateDescriptorSets()
	{
		DescriptorCount descriptorPoolCounts[] =
		{
			{ DescriptorType::CombinedImageSampler, 10000 }
		};
		m_DescriptorPool = m_Context->CreateDescriptorPool(descriptorPoolCounts);

		auto* setLayoutBuilder = m_Context->CreateSetLayoutBuilder();

		{
			auto* materialSetLayout = setLayoutBuilder->Start()
				.Stages(ShaderStage::Vertex | ShaderStage::Fragment)
				.Binding(256, DescriptorType::CombinedImageSampler)
				.Build();
			m_MaterialDescriptorSet = m_DescriptorPool->AllocateSet(materialSetLayout);

			m_Sampler = m_Context->CreateSampler();
		}

		m_Context->DestroySetLayoutBuilder(setLayoutBuilder);
	}

	void SceneRenderer::BuildPipelines()
	{
		auto* pipelineBuilder = m_Context->CreateGraphicsPipelineBuilder();

		{
			auto shader = m_Context->GetShaderCompiler()->CompileFromFile("Resources/Shaders/Geometry.glsl");
			m_MeshPipeline = pipelineBuilder->Start()
				.WithShader(shader)
				.ColorAttachment(ImageFormat::BGRA8UNorm)
				.DepthAttachment()
				.AddVertexInput(0, ShaderDataType::Float3)
				.AddVertexInput(1, ShaderDataType::Float3)
				.AddVertexInput(2, ShaderDataType::Float2)
				.AddVertexInput(3, ShaderDataType::UInt)
				.PushConstant(0, sizeof(FrameTransforms))
				.AddDescriptorSetLayout(m_MaterialDescriptorSet->GetLayout())
				.Build();
		}

		m_Context->DestroyGraphicsPipelineBuilder(pipelineBuilder);
	}

}
