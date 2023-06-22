#pragma once

#include "Yuki/Rendering/RenderAPI.hpp"
#include "Yuki/Rendering/ImageFormat.hpp"
#include "Yuki/Rendering/RHI/CommandBufferPool.hpp"
#include "Yuki/Rendering/RHI/Buffer.hpp"
#include "Yuki/Rendering/RHI/DescriptorSet.hpp"

#include "Yuki/Memory/Unique.hpp"

#include <span>

namespace Yuki {

	class GenericWindow;
	class Viewport;
	class ShaderManager;
	class ShaderCompiler;
	class GraphicsPipelineBuilder;
	class SetLayoutBuilder;
	class Image2D;
	class ImageView2D;
	class RenderInterface;
	class Queue;
	class Fence;
	class Sampler;

	class RenderContext
	{
	public:
		virtual ~RenderContext() = default;

		virtual void Initialize() = 0;
		virtual void Destroy() = 0;

		virtual ShaderManager* GetShaderManager() const = 0;
		virtual ShaderCompiler* GetShaderCompiler() const = 0;

		virtual Queue* GetGraphicsQueue() const = 0;

		virtual void WaitDeviceIdle() const = 0;

	public:
		virtual Unique<RenderInterface> CreateRenderInterface() = 0;
		virtual Unique<GraphicsPipelineBuilder> CreateGraphicsPipelineBuilder() = 0;
		virtual Unique<SetLayoutBuilder> CreateSetLayoutBuilder() = 0;
		virtual Unique<DescriptorPool> CreateDescriptorPool(std::span<DescriptorCount> InDescriptorCounts) = 0;
		virtual Unique<Viewport> CreateViewport(GenericWindow* InWindow) = 0;
		virtual Unique<Image2D> CreateImage2D(uint32_t InWidth, uint32_t InHeight, ImageFormat InFormat, ImageUsage InUsage) = 0;
		virtual Unique<ImageView2D> CreateImageView2D(Image2D* InImage) = 0;
		virtual Unique<Sampler> CreateSampler() = 0;
		virtual Unique<Buffer> CreateBuffer(const BufferInfo& InInfo) = 0;
		virtual Unique<Fence> CreateFence() = 0;
		virtual Unique<CommandBufferPool> CreateCommandBufferPool(CommandBufferPoolInfo InInfo) = 0;

	public:
		static Unique<RenderContext> New(RenderAPI InRenderAPI);
	};

}
