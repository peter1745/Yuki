#include "SpriteRenderer.hpp"

#include <rtmcpp/PackedVector.hpp>

namespace Yuki {

	struct PushConstants
	{
		rtmcpp::PackedMat4 ViewProjection;
		uint64_t Positions;
		uint64_t Transforms;
		uint64_t SpritesBuffer;
	} PC;

	struct Vertex
	{
		rtmcpp::PackedVec2 Position;
	};

	SpriteRenderer::SpriteRenderer(RHIContext context, Aura::Span<ShaderConfig> shaders)
		: m_Context(context)
	{
		m_GraphicsQueue = context.RequestQueue(QueueType::Graphics);
		m_TransferQueue = context.RequestQueue(QueueType::Transfer);

		m_CommandPool = CommandPool::Create(context, m_GraphicsQueue);

		m_UploadFence = Fence::Create(context);

		m_Pipeline = GraphicsPipeline::Create(context, {
			.Shaders = { shaders.Begin(), shaders.End() },
			.PushConstantSize = sizeof(PushConstants),
			.ColorAttachmentFormats = {
				ImageFormat::RGBA8Unorm
			}
		});

		auto vertices = std::array
		{
			Vertex{{ -8.0f,  8.0f }},
			Vertex{{  8.0f,  8.0f }},
			Vertex{{  8.0f, -8.0f }},
			Vertex{{ -8.0f, -8.0f }},
		};

		static constexpr uint32_t MaxSprites = 1024 * 10;

		auto indices = std::array{ 0, 1, 2, 2, 3, 0 };

		m_StorageBuffer = Buffer::Create(context, 10 * 1024 * 1024, BufferUsage::TransferSrc | BufferUsage::Mapped);
		m_StorageBuffer.Set(Aura::Span{ vertices.data(), vertices.size() });
		m_StorageBuffer.Set(Aura::Span{ indices.data(), indices.size() }, 4 * sizeof(Vertex));

		m_VertexBuffer = Buffer::Create(context, 4 * sizeof(Vertex), BufferUsage::StorageBuffer | BufferUsage::TransferDst);
		m_IndexBuffer = Buffer::Create(context, 6 * sizeof(uint32_t), BufferUsage::IndexBuffer | BufferUsage::TransferDst);

		m_TransformsBuffer = Buffer::Create(context, MaxSprites * sizeof(rtmcpp::PackedMat4), BufferUsage::StorageBuffer | BufferUsage::TransferDst);
		m_TransformsStorageBuffer = Buffer::Create(context, MaxSprites * sizeof(rtmcpp::PackedMat4), BufferUsage::TransferSrc | BufferUsage::Mapped);

		m_SpriteDataBuffer = Buffer::Create(context, MaxSprites * sizeof(SpriteData), BufferUsage::StorageBuffer | BufferUsage::TransferDst);
		m_SpriteDataStagingBuffer = Buffer::Create(context, MaxSprites * sizeof(SpriteData), BufferUsage::TransferSrc | BufferUsage::Mapped);

		PC.Positions = m_VertexBuffer.GetAddress();
		PC.Transforms = m_TransformsBuffer.GetAddress();
		PC.SpritesBuffer = m_SpriteDataBuffer.GetAddress();
	}

	static uint32_t writeIndex = 0;

	void SpriteRenderer::Begin(const rtmcpp::PackedMat4& viewProjection)
	{
		m_Sprites.clear();
		writeIndex = 0;

		PC.ViewProjection = viewProjection;
	}

	void SpriteRenderer::Submit(const rtmcpp::PackedMat4& transform, const SpriteData& spriteData)
	{
		m_TransformsStorageBuffer.Set(Aura::Span<rtmcpp::PackedMat4>{ transform }, writeIndex * sizeof(rtmcpp::PackedMat4));
		m_SpriteDataStagingBuffer.Set(Aura::Span<SpriteData>{ spriteData }, writeIndex * sizeof(SpriteData));
		m_Sprites.push_back({ spriteData, transform });
		writeIndex++;
	}

	void SpriteRenderer::End()
	{
	}

	bool m_FirstFrame = true;

	void SpriteRenderer::Render(Fence fence)
	{
		RenderingAttachment attachment = { m_FinalImage.GetDefaultView() };

		m_CommandPool.Reset();

		if (!m_InitializedBuffers)
		{
			auto copyCmd = m_CommandPool.NewList();
			copyCmd.CopyBuffer(m_VertexBuffer, m_StorageBuffer, 4 * sizeof(Vertex));
			copyCmd.CopyBuffer(m_IndexBuffer, m_StorageBuffer, 6 * sizeof(uint32_t), 4 * sizeof(Vertex));
			m_TransferQueue.SubmitCommandLists({ copyCmd }, {}, { m_UploadFence });
			m_UploadValue = m_UploadFence.GetValue();
			m_InitializedBuffers = true;
		}

		auto cmd = m_CommandPool.NewList();
		cmd.TransitionImage(m_FinalImage, ImageLayout::AttachmentOptimal);

		if (writeIndex > 0 && m_FirstFrame)
		{
			cmd.CopyBuffer(m_TransformsBuffer, m_TransformsStorageBuffer, writeIndex * sizeof(rtmcpp::PackedMat4));
			cmd.CopyBuffer(m_SpriteDataBuffer, m_SpriteDataStagingBuffer, writeIndex * sizeof(SpriteData));
			m_FirstFrame = false;
		}

		cmd.BeginRendering({ attachment });

		if (m_UploadFence.GetCurrentValue() == m_UploadValue)
		{
			cmd.BindPipeline(m_Pipeline);
			cmd.SetViewports({ m_Viewport });
			cmd.BindIndexBuffer(m_IndexBuffer);
			cmd.SetPushConstants(m_Pipeline, PC);

			for (uint32_t i = 0; i < m_Sprites.size(); i++)
			{
				cmd.DrawIndexed(6, i);
			}
		}

		cmd.EndRendering();
		cmd.TransitionImage(m_FinalImage, ImageLayout::TransferSrc);

		m_GraphicsQueue.SubmitCommandLists({ cmd }, { fence }, { fence });
	}

	void SpriteRenderer::SetSize(uint32_t width, uint32_t height)
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
