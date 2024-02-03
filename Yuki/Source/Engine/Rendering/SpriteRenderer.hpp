#pragma once

#include "Engine/RHI/RHI.hpp"

#include <rtmcpp/Matrix.hpp>
#include <rtmcpp/PackedMatrix.hpp>

#include <vector>

namespace Yuki {

	struct SpriteData
	{
		uint32_t Color;
	};

	class SpriteRenderer
	{
	public:
		SpriteRenderer(RHIContext context, Aura::Span<ShaderConfig> shaders);

		void Begin(const rtmcpp::PackedMat4& viewProjection);
		void Submit(const rtmcpp::PackedMat4& transform, const SpriteData& spriteData);
		void End();

		void Render(Fence fence);

		void SetSize(uint32_t width, uint32_t height);

		Image GetFinalImage() const { return m_FinalImage; }

	private:
		struct GeometryData
		{
			SpriteData Data;
			rtmcpp::PackedMat4 Transform;
		};

		std::vector<GeometryData> m_Sprites;

		RHIContext m_Context;
		Queue m_GraphicsQueue, m_TransferQueue;
		Fence m_UploadFence;
		uint32_t m_UploadValue;

		GraphicsPipeline m_Pipeline;
		CommandPool m_CommandPool;

		Image m_FinalImage;
		Viewport m_Viewport;

		Buffer m_StorageBuffer, m_VertexBuffer, m_IndexBuffer;
		Buffer m_TransformsBuffer, m_TransformsStorageBuffer;
		Buffer m_SpriteDataBuffer, m_SpriteDataStagingBuffer;
		bool m_InitializedBuffers = false;
	};

}
