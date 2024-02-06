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
		rtmcpp::PackedVec2 UV;
		uint32_t Color;
		uint32_t Texture = ~0u;
	};

	static constexpr float32_t QuadHalfSize = 8.0f;

	template<>
	struct Handle<GeometryBatch>::Impl
	{
		RHIContext Context;

		std::vector<BatchedVertex> Vertices;
		std::vector<uint32_t> Indices;
		uint32_t BaseIndex = 0;

		Buffer VertexBuffer;
		Buffer IndexBuffer;

		std::vector<Image> Images;

		bool IsDirty = false;

		void CreateResources()
		{
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

		m_DescriptorHeap = DescriptorHeap::Create(context);

		m_Pipeline = GraphicsPipeline::Create(context, {
			.Shaders = { shaders.Begin(), shaders.End() },
			.PushConstantSize = sizeof(BatchPushConstants),
			.ColorAttachmentFormats = {
				ImageFormat::RGBA8Unorm
			}
		}, m_DescriptorHeap);

		m_DefaultSampler = Sampler::Create(context, {
			.MinFilter = ImageFilter::Nearest,
			.MagFilter = ImageFilter::Nearest,
			.WrapMode = ImageWrapMode::Repeat
		});

		m_DescriptorHeap.WriteSampler(0, m_DefaultSampler);
		
		// NOTE(Peter): Growable staging buffer (or several smaller staging buffers?)
 		m_StagingBuffer = Buffer::Create(context, 10 * 1024 * 1024, BufferUsage::TransferSrc | BufferUsage::Mapped);
	}

	GeometryBatch BatchRenderer::NewBatch()
	{
		auto* batch = new GeometryBatch::Impl();
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

				batch->CreateResources();

				for (uint32_t i = 0; i < batch->Images.size(); i++)
				{
					m_DescriptorHeap.WriteSampledImage(i, batch->Images[i].GetDefaultView());
				}

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
		cmd.BindDescriptorHeap(m_DescriptorHeap, m_Pipeline);
		cmd.SetViewports({ m_Viewport });

		for (auto batch : m_Batches)
		{
			if (!batch->VertexBuffer || !batch->IndexBuffer)
			{
				continue;
			}

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

	void GeometryBatch::Clear() const
	{
		m_Impl->Vertices.clear();
		m_Impl->Indices.clear();
		m_Impl->BaseIndex = 0;

		if (m_Impl->VertexBuffer)
		{
			m_Impl->VertexBuffer.Destroy();
		}

		if (m_Impl->IndexBuffer)
		{
			m_Impl->IndexBuffer.Destroy();
		}

		m_Impl->Images.clear();
	}

	void GeometryBatch::AddQuad(rtmcpp::Vec2 position, rtmcpp::Vec4 color) const
	{
		// Add new vertices
		{
			m_Impl->Vertices.push_back({
				.Position = {  position.X - QuadHalfSize, position.Y + QuadHalfSize },
				.Color = rtmcpp::PackUnorm4x8<float>(color)
			});

			m_Impl->Vertices.push_back({
				.Position = {  position.X + QuadHalfSize, position.Y + QuadHalfSize },
				.Color = rtmcpp::PackUnorm4x8<float>(color)
			});

			m_Impl->Vertices.push_back({
				.Position = {  position.X + QuadHalfSize, position.Y - QuadHalfSize },
				.Color = rtmcpp::PackUnorm4x8<float>(color)
			});

			m_Impl->Vertices.push_back({
				.Position = {  position.X - QuadHalfSize, position.Y - QuadHalfSize },
				.Color = rtmcpp::PackUnorm4x8<float>(color)
			});
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

	void GeometryBatch::AddTexturedQuad(rtmcpp::Vec2 position, Image image) const
	{
		uint32_t imageIndex = ~0u;

		for (uint32_t i = 0; i < m_Impl->Images.size(); i++)
		{
			if (m_Impl->Images[i] == image)
			{
				imageIndex = i;
				break;
			}
		}

		if (imageIndex == ~0u)
		{
			imageIndex = static_cast<uint32_t>(m_Impl->Images.size());
			m_Impl->Images.push_back(image);
		}

		// Add new vertices
		{
			m_Impl->Vertices.push_back({
				.Position = {  position.X - QuadHalfSize, position.Y + QuadHalfSize },
				.UV = { 0.0f, 1.0f },
				.Texture = imageIndex
			});

			m_Impl->Vertices.push_back({ 
				.Position = {  position.X + QuadHalfSize, position.Y + QuadHalfSize },
				.UV = { 1.0f, 1.0f },
				.Texture = imageIndex
			});

			m_Impl->Vertices.push_back({ 
				.Position = {  position.X + QuadHalfSize, position.Y - QuadHalfSize },
				.UV = { 1.0f, 0.0f },
				.Texture = imageIndex
			});

			m_Impl->Vertices.push_back({ 
				.Position = {  position.X - QuadHalfSize, position.Y - QuadHalfSize },
				.UV = { 0.0f, 0.0f },
				.Texture = imageIndex
			});
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

	void GeometryBatch::MarkDirty() const
	{
		m_Impl->IsDirty = true;
	}

}
