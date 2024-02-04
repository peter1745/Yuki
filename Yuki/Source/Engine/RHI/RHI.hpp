#pragma once

#include "Engine/Core/Core.hpp"
#include "Engine/Core/Handle.hpp"
#include "Engine/Core/EnumFlags.hpp"

#include <Aura/Span.hpp>

#include <filesystem>
#include <functional>

namespace Yuki {

	enum class QueueType
	{
		Graphics = 1 << 0,
		Compute  = 1 << 1,
		Transfer = 1 << 2
	};

	struct Queue;

	struct RHIContext : Handle<RHIContext>
	{
		static RHIContext Create();
		void Destroy();

		Aura::Span<Queue> RequestQueues(QueueType type, uint32_t count) const;
		Queue RequestQueue(QueueType type) const;
	};

	enum class ImageLayout
	{
		Undefined,
		General,
		AttachmentOptimal,
		TransferSrc,
		TransferDst,
		ShaderReadOnlyOptimal,
		Present,
	};

	enum class ImageFormat
	{
		RGBA8Unorm,
		BGRA8Unorm,
	};

	enum class ImageUsage
	{
		ColorAttachment        = 1 << 0,
		DepthStencilAttachment = 1 << 1,
		TransferSrc            = 1 << 2,
		TransferDst            = 1 << 3,
		Sampled                = 1 << 4,
	};
	inline void MakeEnumFlags(ImageUsage) {}

	struct ImageConfig
	{
		uint32_t Width;
		uint32_t Height;
		ImageFormat Format;
		ImageUsage Usage;
		bool CreateDefaultView = false;
	};

	struct ImageView;
	struct Image : Handle<Image>
	{
		static Image Create(RHIContext context, const ImageConfig& config);
		void Destroy();

		ImageView GetDefaultView() const;
	};

	struct ImageView : Handle<ImageView>
	{
		static ImageView Create(RHIContext context, Image image);
		void Destroy();
	};

	class Window;
	struct Swapchain : Handle<Swapchain>
	{
		static Swapchain Create(RHIContext context, Window window);
		void Destroy();

		Image GetCurrentImage() const;
		ImageView GetCurrentImageView() const;
	};

	struct Fence : Handle<Fence>
	{
		static Fence Create(RHIContext context);
		void Destroy();

		uint64_t GetValue() const;
		uint64_t GetCurrentValue() const;

		void Wait(uint64_t value = 0) const;
	};

	struct CommandList;
	struct Queue : Handle<Queue>
	{
		void Destroy() {}

		void AcquireImages(Aura::Span<Swapchain> swapchains, Aura::Span<Fence> signals) const;
		void SubmitCommandLists(Aura::Span<CommandList> commandLists, Aura::Span<Fence> waits, Aura::Span<Fence> signals) const;
		void Present(Aura::Span<Swapchain> swapchains, Aura::Span<Fence> waits) const;
	};

	enum class ImageFilter
	{
		Nearest,
		Linear,
		Cubic
	};

	enum class ImageWrapMode
	{
		Repeat,
		MirroredRepeat,
		ClampToEdge,
		ClampToBorder,
		MirrorClampToEdge
	};

	struct SamplerConfig
	{
		ImageFilter MinFilter = ImageFilter::Linear;
		ImageFilter MagFilter = ImageFilter::Linear;
		ImageWrapMode WrapMode = ImageWrapMode::ClampToEdge;
	};

	struct Sampler : Handle<Sampler>
	{
		static Sampler Create(RHIContext context, const SamplerConfig& config);
		void Destroy();
	};

	struct DescriptorHeap : Handle<DescriptorHeap>
	{
		static DescriptorHeap Create(RHIContext context);
		void Destroy();

		void WriteSampledImage(uint32_t index, ImageView imageView);
		void WriteSampler(uint32_t index, Sampler sampler);
	};

	enum class ShaderStage
	{
		Vertex, Fragment
	};

	struct ShaderConfig
	{
		ShaderStage Stage;
		std::filesystem::path FilePath;
	};

	struct GraphicsPipelineConfig
	{
		std::vector<ShaderConfig> Shaders;
		uint32_t PushConstantSize;

		std::vector<ImageFormat> ColorAttachmentFormats;
	};

	struct GraphicsPipeline : Handle<GraphicsPipeline>
	{
		static GraphicsPipeline Create(RHIContext context, const GraphicsPipelineConfig& config, DescriptorHeap heap);
		void Destroy();
	};

	enum class BufferUsage
	{
		TransferSrc     = 1 << 0,
		TransferDst     = 1 << 1,
		UniformBuffer   = 1 << 2,
		StorageBuffer   = 1 << 3,
		IndexBuffer     = 1 << 4,
		VertexBuffer    = 1 << 5,
		Mapped          = 1 << 6,
		DeviceLocal     = 1 << 7,
		DedicatedMemory = 1 << 8,
	};
	inline void MakeEnumFlags(BufferUsage) {}

	struct Buffer : Handle<Buffer>
	{
		static Buffer Create(RHIContext context, uint64_t size, BufferUsage usage);
		void Destroy();

		uint64_t GetAddress() const;

		void SetData(const std::byte* data, uint32_t offset, uint32_t size) const;

		template<typename T>
		void Set(Aura::Span<T> data, uint32_t offset = 0)
		{
			SetData(reinterpret_cast<std::byte*>(data.Data()), offset, data.ByteCount());
		}
	};

	struct RenderingAttachment
	{
		ImageView Target;
	};

	struct Viewport
	{
		uint32_t Width;
		uint32_t Height;
	};

	struct CommandList : Handle<CommandList>
	{
		void BeginRendering(Aura::Span<RenderingAttachment> colorAttachments) const;
		void EndRendering() const;

		void SetViewports(Aura::Span<Viewport> viewports) const;
		void BindPipeline(GraphicsPipeline pipeline) const;
		
		void TransitionImage(Image image, ImageLayout layout) const;
		void BlitImage(Image dest, Image src) const;

		void BindDescriptorHeap(DescriptorHeap heap, GraphicsPipeline pipeline) const;
		void BindVertexBuffer(Buffer buffer, uint32_t stride) const;
		void BindIndexBuffer(Buffer buffer) const;

		void CopyBuffer(Buffer dest, Buffer src, uint32_t size, uint32_t srcOffset = 0, uint32_t destOffset = 0) const;
		void CopyBufferToImage(Image dest, Buffer src, uint32_t size, uint32_t srcOffset = 0) const;

		void SetPushConstants(GraphicsPipeline pipeline, const void* data, uint32_t size) const;

		template<typename T>
		void SetPushConstants(GraphicsPipeline pipeline, const T& data) const
		{
			SetPushConstants(pipeline, &data, sizeof(T));
		}

		void Draw(uint32_t vertexCount) const;
		void DrawIndexed(uint32_t indexCount, uint32_t instanceIndex) const;
	};

	struct CommandPool : Handle<CommandPool>
	{
		static CommandPool Create(RHIContext context, Queue queue);
		void Destroy();

		void Reset() const;

		CommandList NewList() const;
	};
}
