#pragma once

#include "Engine/RHI/RHI.hpp"

#include <rtmcpp/Vector.hpp>
#include <rtmcpp/Matrix.hpp>

namespace Yuki {

	struct RenderBatch : Handle<RenderBatch>
	{
		void AddQuad(rtmcpp::Vec2 position, rtmcpp::Vec4 color) const;
		void MarkDirty() const;
	};

	class BatchRenderer
	{
	public:
		BatchRenderer(RHIContext context, Aura::Span<ShaderConfig> shaders);

		RenderBatch NewBatch();

		void Render(const rtmcpp::Mat4& viewProjection, Fence fence);

		void SetSize(uint32_t width, uint32_t height);
		Image GetFinalImage() const { return m_FinalImage; }

	private:
		RHIContext m_Context;
		Queue m_GraphicsQueue, m_TransferQueue;
		Fence m_UploadFence;

		GraphicsPipeline m_Pipeline;
		CommandPool m_CommandPool;

		Image m_FinalImage;
		Viewport m_Viewport;

		Buffer m_StagingBuffer;

		std::vector<RenderBatch> m_Batches;
	};

}
