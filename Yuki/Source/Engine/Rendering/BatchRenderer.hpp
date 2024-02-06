#pragma once

#include "GeometryBatch.hpp"
#include "Engine/RHI/RHI.hpp"

#include <rtmcpp/Vector.hpp>
#include <rtmcpp/Matrix.hpp>

namespace Yuki {

	class BatchRenderer
	{
	public:
		BatchRenderer(RHIContext context, Aura::Span<ShaderConfig> shaders);

		GeometryBatch NewBatch();

		void Render(const rtmcpp::Mat4& viewProjection, Fence fence);

		void SetSize(uint32_t width, uint32_t height);
		Image GetFinalImage() const { return m_FinalImage; }

	private:
		RHIContext m_Context;
		Queue m_GraphicsQueue, m_TransferQueue;
		Fence m_UploadFence;

		DescriptorHeap m_DescriptorHeap;
		Sampler m_DefaultSampler;

		GraphicsPipeline m_Pipeline;
		CommandPool m_CommandPool;

		Image m_FinalImage;
		Viewport m_Viewport;

		Buffer m_StagingBuffer;

		std::vector<GeometryBatch> m_Batches;
	};

}
