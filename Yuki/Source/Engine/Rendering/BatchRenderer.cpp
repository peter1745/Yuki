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

	static constexpr uint32_t MaxQuads = 1'000'000;
	static constexpr uint32_t MaxVertices = MaxQuads * 4;
	static constexpr uint32_t VertexBufferSize = MaxVertices * sizeof(BatchedVertex);
	static constexpr uint32_t MaxIndices = MaxQuads * 6;
	static constexpr uint32_t IndexBufferSize = MaxIndices * sizeof(uint32_t);

	static std::array<BatchedVertex, 4> CreateQuad(rtmcpp::Vec2 position, rtmcpp::Vec4 color)
	{
		return {
			BatchedVertex{{  position.X - 16.0f, position.Y + 16.0f }, rtmcpp::PackUnorm4x8<float>(color) },
			BatchedVertex{{  position.X + 16.0f, position.Y + 16.0f }, rtmcpp::PackUnorm4x8<float>(color) },
			BatchedVertex{{  position.X + 16.0f, position.Y - 16.0f }, rtmcpp::PackUnorm4x8<float>(color) },
			BatchedVertex{{  position.X - 16.0f, position.Y - 16.0f }, rtmcpp::PackUnorm4x8<float>(color) },
		};
	}

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

		uint32_t baseIndex = 0;

		std::vector<uint32_t> indices;
		indices.reserve(MaxIndices);

		for (uint32_t i = 0; i < MaxQuads; i++)
		{
			indices.push_back(baseIndex + 0);
			indices.push_back(baseIndex + 1);
			indices.push_back(baseIndex + 2);

			indices.push_back(baseIndex + 2);
			indices.push_back(baseIndex + 3);
			indices.push_back(baseIndex + 0);

			baseIndex += 4;
		}

 		m_StagingBuffer = Buffer::Create(context, VertexBufferSize + IndexBufferSize, BufferUsage::TransferSrc | BufferUsage::Mapped);
		m_StagingBuffer.Set(Aura::Span{ indices.data(), static_cast<uint32_t>(indices.size()) }, VertexBufferSize);

		m_VertexBuffer = Buffer::Create(context, VertexBufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst | BufferUsage::DedicatedMemory);
		m_VertexBackBuffer = Buffer::Create(context, VertexBufferSize, BufferUsage::StorageBuffer | BufferUsage::TransferDst | BufferUsage::DedicatedMemory);
		m_CurrentVertexBuffer = m_VertexBuffer;
		PC.Vertices = m_VertexBuffer.GetAddress();

		m_IndexBuffer = Buffer::Create(context, IndexBufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst | BufferUsage::DedicatedMemory);
		m_IndexBackBuffer = Buffer::Create(context, IndexBufferSize, BufferUsage::IndexBuffer | BufferUsage::TransferDst | BufferUsage::DedicatedMemory);
		m_CurrentIndexBuffer = m_IndexBuffer;

		WriteLine("Allocated {}MB for BatchRenderer", (VertexBufferSize + IndexBufferSize) / (1024 * 1024));
	}

	void BatchRenderer::DrawQuad(rtmcpp::Vec2 position, rtmcpp::Vec4 color)
	{
		auto vertices = CreateQuad(position, color);
		m_StagingBuffer.Set(Aura::Span{ vertices.data(), vertices.size() }, m_VertexCount * sizeof(BatchedVertex));
		m_VertexCount += vertices.size();
		m_IndexCount += 6;
		m_VertexBufferDirty = true;
	}

	void BatchRenderer::Render(const rtmcpp::Mat4& viewProjection, Fence fence)
	{
		PC.ViewProjection = viewProjection;

		RenderingAttachment attachment = { m_FinalImage.GetDefaultView() };

		m_CommandPool.Reset();

		if (m_VertexBufferDirty)
		{
			auto copyCmd = m_CommandPool.NewList();
			copyCmd.CopyBuffer(m_VertexBuffer, m_StagingBuffer, m_VertexCount * sizeof(BatchedVertex));
			copyCmd.CopyBuffer(m_IndexBuffer, m_StagingBuffer, m_IndexCount * sizeof(uint32_t), VertexBufferSize);
			m_TransferQueue.SubmitCommandLists({ copyCmd }, {}, { m_UploadFence });
			m_VertexBufferDirty = false;
		}

		m_UploadFence.Wait();

		auto cmd = m_CommandPool.NewList();
		cmd.TransitionImage(m_FinalImage, ImageLayout::AttachmentOptimal);
		cmd.BeginRendering({ attachment });
		cmd.BindPipeline(m_Pipeline);
		cmd.SetViewports({ m_Viewport });
		cmd.BindIndexBuffer(m_IndexBuffer);
		cmd.SetPushConstants(m_Pipeline, PC);
		cmd.DrawIndexed(m_IndexCount, 0);
		cmd.EndRendering();
		cmd.TransitionImage(m_FinalImage, ImageLayout::TransferSrc);

		m_GraphicsQueue.SubmitCommandLists({ cmd }, { fence }, { fence });

		m_VertexCount = 0;
		m_IndexCount = 0;
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

}
