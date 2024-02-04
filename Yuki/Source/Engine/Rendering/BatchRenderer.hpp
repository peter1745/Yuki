#pragma once

#include "Engine/RHI/RHI.hpp"

#include <rtmcpp/Vector.hpp>
#include <rtmcpp/Matrix.hpp>

namespace Yuki {

	class BatchRenderer
	{
	public:
		BatchRenderer(RHIContext context, Aura::Span<ShaderConfig> shaders);

		void DrawQuad(rtmcpp::Vec2 position, rtmcpp::Vec4 color);

		void Render(const rtmcpp::Mat4& viewProjection, Fence fence);

		void SetSize(uint32_t width, uint32_t height);

		Image GetFinalImage() const { return m_FinalImage; }

	private:
		RHIContext m_Context;
		Queue m_GraphicsQueue, m_TransferQueue;
		Fence m_UploadFence;
		uint32_t m_UploadValue;

		GraphicsPipeline m_Pipeline;
		CommandPool m_CommandPool;

		Image m_FinalImage;
		Viewport m_Viewport;

		Buffer m_VertexBuffer, m_VertexBackBuffer, m_CurrentVertexBuffer;
		uint32_t m_VertexCount = 0;
		bool m_VertexBufferDirty = false;

		Buffer m_IndexBuffer, m_IndexBackBuffer, m_CurrentIndexBuffer;
		uint32_t m_IndexCount = 0;


		Buffer m_StagingBuffer;
	};

}
