#include "BatchRenderer.hpp"

#include <rtmcpp/PackedVector.hpp>
#include <rtmcpp/VectorOps.hpp>
#include <rtmcpp/PackedMatrix.hpp>

namespace Yuki {

	struct BatchPushConstants
	{
		rtmcpp::PackedMat4 ViewProjection;
		uint64_t Vertices;
	} PC;

	struct BatchedVertex
	{
		rtmcpp::PackedVec2 Position;
		uint32_t Color;
	};

	template<>
	struct Handle<RenderBatch>::Impl
	{
		RHIContext Context;

		std::vector<BatchedVertex> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t BaseIndex = 0;

		Buffer VertexBuffer;
		Buffer IndexBuffer;

		bool IsDirty = false;

		void CreateBuffers()
		{
			if (VertexBuffer)
				VertexBuffer.Destroy();

			if (IndexBuffer)
				IndexBuffer.Destroy();

			VertexBuffer = Buffer::Create(Context, Vertices.size() * sizeof(BatchedVertex), BufferUsage::StorageBuffer | BufferUsage::TransferDst);
			IndexBuffer = Buffer::Create(Context, Indices.size() * sizeof(uint32_t), BufferUsage::IndexBuffer | BufferUsage::TransferDst);
		}
	};

	BatchRenderer::BatchRenderer(RHIContext context, Aura::Span<ShaderConfig> shaders)
		: m_Context(context)
	{
		m_GraphicsQueue = context.RequestQueue(QueueType::Graphics);
		m_TransferQueue = context.RequestQueue(QueueType::Transfer);

		m_CommandPool = CommandPool::Create(context, m_GraphicsQueue);

		m_UploadFence = Fence::Create(context);

		m_Pipeline = GraphicsPipeline::Create(context, {
			.Shaders = { shaders.Begin(), shaders.End() },
			.PushConstantSize = sizeof(BatchPushConstants),
			.ColorAttachmentFormats = {
				ImageFormat::RGBA8Unorm
			}
		});

 		m_StagingBuffer = Buffer::Create(context, 100 * 1024 * 1024, BufferUsage::TransferSrc | BufferUsage::Mapped);
	}

	RenderBatch BatchRenderer::NewBatch()
	{
		auto* batch = new RenderBatch::Impl();
		batch->Context = m_Context;
		m_Batches.push_back({ batch });
		return { batch };
	}

	void BatchRenderer::Render(const rtmcpp::Mat4& viewProjection, Fence fence)
	{
		PC.ViewProjection = viewProjection;

		RenderingAttachment attachment = { m_FinalImage.GetDefaultView() };

		m_CommandPool.Reset();

		uint32_t stagingOffset = 0;

		CommandList copyCmd = {};

		for (auto batch : m_Batches)
		{
			if (batch->IsDirty)
			{
				if (!copyCmd)
				{
					copyCmd = m_CommandPool.NewList();
				}

				batch->CreateBuffers();

				uint32_t vertexSize = static_cast<uint32_t>(batch->Vertices.size()) * sizeof(BatchedVertex);
				uint32_t indexSize = static_cast<uint32_t>(batch->Indices.size()) * sizeof(uint32_t);

				m_StagingBuffer.Set(Aura::Span{ batch->Vertices.data(), static_cast<uint32_t>(batch->Vertices.size()) }, stagingOffset);
				copyCmd.CopyBuffer(batch->VertexBuffer, m_StagingBuffer, vertexSize, stagingOffset);
				stagingOffset += vertexSize;
				m_StagingBuffer.Set(Aura::Span{ batch->Indices.data(), static_cast<uint32_t>(batch->Indices.size()) }, stagingOffset);
				copyCmd.CopyBuffer(batch->IndexBuffer, m_StagingBuffer, indexSize, stagingOffset);
				stagingOffset += indexSize;

				batch->IsDirty = false;
			}
		}

		if (copyCmd)
		{
			m_TransferQueue.SubmitCommandLists({ copyCmd }, {}, { m_UploadFence });

			// NOTE(Peter): Potentially quite slow, horrible regardless, consider double-buffering
			m_UploadFence.Wait();
		}

		auto cmd = m_CommandPool.NewList();
		cmd.TransitionImage(m_FinalImage, ImageLayout::AttachmentOptimal);
		cmd.BeginRendering({ attachment });
		cmd.BindPipeline(m_Pipeline);
		cmd.SetViewports({ m_Viewport });

		for (auto batch : m_Batches)
		{
			PC.Vertices = batch->VertexBuffer.GetAddress();
			cmd.SetPushConstants(m_Pipeline, PC);
			cmd.BindIndexBuffer(batch->IndexBuffer);
			cmd.DrawIndexed(static_cast<uint32_t>(batch->Indices.size()), 0);
		}

		cmd.EndRendering();
		cmd.TransitionImage(m_FinalImage, ImageLayout::TransferSrc);

		m_GraphicsQueue.SubmitCommandLists({ cmd }, { fence }, { fence });
	}

	void BatchRenderer::SetSize(uint32_t width, uint32_t height)
	{
		if (m_FinalImage)
		{
			m_FinalImage.Destroy();
		}

		m_FinalImage = Image::Create(m_Context, {
			.Width = width,
			.Height = height,
			.Format = ImageFormat::RGBA8Unorm,
			.Usage = ImageUsage::ColorAttachment | ImageUsage::TransferSrc,
			.CreateDefaultView = true
		});

		m_Viewport = { width, height };
	}

	void RenderBatch::AddQuad(rtmcpp::Vec2 position, rtmcpp::Vec4 color) const
	{
		// Add new vertices
		{
			m_Impl->Vertices.push_back({ {  position.X - 8.0f, position.Y + 8.0f }, rtmcpp::PackUnorm4x8<float>(color) });
			m_Impl->Vertices.push_back({ {  position.X + 8.0f, position.Y + 8.0f }, rtmcpp::PackUnorm4x8<float>(color) });
			m_Impl->Vertices.push_back({ {  position.X + 8.0f, position.Y - 8.0f }, rtmcpp::PackUnorm4x8<float>(color) });
			m_Impl->Vertices.push_back({ {  position.X - 8.0f, position.Y - 8.0f }, rtmcpp::PackUnorm4x8<float>(color) });
		}

		// Add new indices
		{
			m_Impl->Indices.push_back(m_Impl->BaseIndex + 0);
			m_Impl->Indices.push_back(m_Impl->BaseIndex + 1);
			m_Impl->Indices.push_back(m_Impl->BaseIndex + 2);
		
			m_Impl->Indices.push_back(m_Impl->BaseIndex + 2);
			m_Impl->Indices.push_back(m_Impl->BaseIndex + 3);
			m_Impl->Indices.push_back(m_Impl->BaseIndex + 0);

			m_Impl->BaseIndex += 4;
		}
	}

	void RenderBatch::MarkDirty() const
	{
		m_Impl->IsDirty = true;
	}

}
