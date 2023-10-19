#pragma once

#include "Engine/Common/Core.hpp"
#include "Engine/Common/UniqueID.hpp"
#include "Engine/Containers/Span.hpp"

#include "RenderFeatures.hpp"

#include <filesystem>

namespace Yuki {

	class WindowSystem;
	class RenderGraph;

}

namespace Yuki::RHI {

	template<typename T>
	struct RenderHandle
	{
		struct Impl;

		RenderHandle() = default;
		RenderHandle(Impl* impl)
			: m_Impl(impl) {}

		operator T() const noexcept { return T(m_Impl); }
		T Unwrap() const noexcept { return T(m_Impl); }

		operator bool() const noexcept { return m_Impl; }
		Impl* operator->() const noexcept { return m_Impl; }
	protected:
		Impl* m_Impl = nullptr;
	};

#define YUKI_RENDER_HANDLE(Type) using Type##RH = RenderHandle<struct Type>;

	YUKI_RENDER_HANDLE(Context);
	YUKI_RENDER_HANDLE(Queue);
	YUKI_RENDER_HANDLE(Swapchain);
	YUKI_RENDER_HANDLE(PipelineLayout);
	YUKI_RENDER_HANDLE(Pipeline);
	YUKI_RENDER_HANDLE(RayTracingPipeline);
	YUKI_RENDER_HANDLE(DescriptorHeap);
	YUKI_RENDER_HANDLE(Fence);
	YUKI_RENDER_HANDLE(CommandPool);
	YUKI_RENDER_HANDLE(CommandList);
	YUKI_RENDER_HANDLE(Image);
	YUKI_RENDER_HANDLE(ImageView);
	YUKI_RENDER_HANDLE(Sampler);
	YUKI_RENDER_HANDLE(Buffer);
	YUKI_RENDER_HANDLE(AccelerationStructure);
	YUKI_RENDER_HANDLE(RenderPass);

	YUKI_FLAG_ENUM(QueueType)
	{
		Graphics = 1 << 0,
		Compute  = 1 << 1,
		Transfer = 1 << 2
	};
	
	enum class ImageFormat
	{
		RGBA8,
		BGRA8,
		D32SFloat
	};
	YUKI_FLAG_ENUM(ImageUsage)
	{
		ColorAttachment = 1 << 0,
		DepthAttachment = 1 << 1,
		Sampled			= 1 << 2,
		TransferDest	= 1 << 3,
		TransferSource	= 1 << 4,
		Storage			= 1 << 5,
		HostTransfer	= 1 << 6,
	};

	enum class ImageLayout
	{
		Undefined = -1,
		General,
		Attachment,
		ShaderReadOnly,
		Present,
		TransferDest,
		TransferSource
	};

	YUKI_FLAG_ENUM(BufferUsage)
	{
		Vertex							= 1 << 0,
		Index							= 1 << 1,
		Storage							= 1 << 2,
		TransferSrc						= 1 << 3,
		TransferDst						= 1 << 4,
		ShaderBindingTable				= 1 << 5,
		AccelerationStructureStorage	= 1 << 6,
		AccelerationStructureBuildInput = 1 << 7,
	};

	YUKI_FLAG_ENUM(BufferFlags)
	{
		None = 0,
		Mapped = 1 << 0,
		DeviceLocal = 1 << 1,
	};

	YUKI_FLAG_ENUM(ShaderStage)
	{
		None = 0,
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		RayGeneration = 1 << 2,
		RayMiss = 1 << 3,
		RayClosestHit = 1 << 4,
		RayAnyHit = 1 << 5,
	};

	YUKI_FLAG_ENUM(CommandPoolFlag)
	{
		None = 0,
		TransientLists = 1 << 0
	};

	enum class PipelineType
	{
		Rasterization, Raytracing
	};

	struct PipelineShaderInfo
	{
		std::filesystem::path FilePath;
		ShaderStage Stage;
	};

	struct PipelineLayoutInfo
	{
		uint32_t PushConstantSize = 0;
	};

	struct PipelineInfo
	{
		PipelineLayoutRH Layout = {};
		Span<PipelineShaderInfo> Shaders;
		struct ColorAttachmentInfo { ImageFormat Format; };
		DynamicArray<ColorAttachmentInfo> ColorAttachments;
	};

	struct RayTracingPipelineInfo
	{
		PipelineLayoutRH Layout = {};
		Span<PipelineShaderInfo> Shaders;
	};

#undef YUKI_RENDER_HANDLE

	enum class AttachmentLoadOp { Load, Clear, DontCare };
	enum class AttachmentStoreOp { Store, DontCare };

	struct RenderTargetAttachment
	{
		ImageViewRH ImageView;
		AttachmentLoadOp LoadOp;
		AttachmentStoreOp StoreOp;
	};

	struct RenderTarget
	{
		Span<RenderTargetAttachment> ColorAttachments;
	};

	struct ImageBarrier
	{
		Span<ImageRH> Images;
		Span<ImageLayout> Layouts;
	};

	struct Context : RenderHandle<Context>
	{
		struct Config
		{
			DynamicArray<RendererFeature> RequestedFeatures;
		};

		static Context Create(Config config);

		bool IsFeatureEnabled(RendererFeature feature) const;

		Queue RequestQueue(QueueType type) const;
	};

	struct Queue : RenderHandle<Queue>
	{
		void AcquireImages(Span<Swapchain> swapchains, Span<Fence> fences) const;
		void Submit(Span<CommandListRH> commandLists, Span<FenceRH> waits, Span<FenceRH> signals) const;
		void Present(Span<Swapchain> swapchains, Span<Fence> fences) const;
	};

	struct Swapchain : RenderHandle<Swapchain>
	{
		static Swapchain Create(Context context, const WindowSystem& windowSystem, UniqueID windowHandle);

		void Recreate() const;

		ImageRH GetCurrentImage();
		ImageViewRH GetCurrentImageView();
	};

	struct Fence : RenderHandle<Fence>
	{
		static Fence Create(Context context);
		void Destroy();

		void Wait(uint64_t value = 0);
	};

	struct CommandPool : RenderHandle<CommandPool>
	{
		static CommandPool Create(Context context, QueueRH queue, CommandPoolFlag flags = CommandPoolFlag::None);
		void Destroy();

		void Reset() const;
		CommandList NewList() const;
	};

	struct Viewport
	{
		float X;
		float Y;
		float Width;
		float Height;
	};

	enum class PipelineBindPoint
	{
		Graphics, RayTracing,
	};

	struct CommandList : RenderHandle<CommandList>
	{
		friend CommandPool;

		void Begin();
		void ImageBarrier(ImageBarrier barrier);
		void BeginRendering(RenderTarget renderTarget);
		void EndRendering();
		void CopyBuffer(BufferRH dest, BufferRH src);
		void CopyImage(Image dest, Image src) const;
		void BlitImage(Image dest, Image src) const;
		void PushConstants(PipelineLayout layout, ShaderStage stages, const void* data, uint32_t dataSize);
		void BindDescriptorHeap(PipelineLayout layout, PipelineBindPoint bindPoint, DescriptorHeap heap);
		void BindPipeline(PipelineRH pipeline);
		void BindPipeline(RayTracingPipelineRH pipeline);
		void BindIndexBuffer(BufferRH buffer);
		void SetViewport(Viewport viewport);
		void DrawIndexed(uint32_t indexCount, uint32_t indexOffset, uint32_t instanceIndex);
		void TraceRays(RayTracingPipelineRH pipeline, uint32_t width, uint32_t height);
		void End();
	};

	struct ImageView;

	struct Image : RenderHandle<Image>
	{
		friend Swapchain;

		static Image Create(Context context, uint32_t width, uint32_t height, ImageFormat format, ImageUsage usage);
		void Destroy();

		uint32_t GetWidth() const;
		uint32_t GetHeight() const;

		void Transition(ImageLayout layout) const;
		void SetData(const void* data) const;

		ImageView GetDefaultView() const;
	};

	struct ImageView : RenderHandle<ImageView>
	{
		static ImageView Create(Context context, ImageRH image);
		void Destroy();
	};

	struct Sampler : RenderHandle<Sampler>
	{
		static Sampler Create(Context context);
		void Destroy();
	};

	struct Buffer : RenderHandle<Buffer>
	{
		static Buffer Create(Context context, uint64_t size, BufferUsage usage, BufferFlags flags = BufferFlags::None);
		void Destroy();

		void SetData(const void* data, uint64_t dataSize, uint32_t offset = 0);
		uint64_t GetDeviceAddress();
		void* GetMappedMemory();

		template<typename T>
		void Set(Span<T> elements, uint32_t startIndex = 0)
		{
			T* data = reinterpret_cast<T*>(GetMappedMemory());
			memcpy(data + startIndex, elements.Data(), elements.ByteSize());
		}
	};

	struct DescriptorHeap : RenderHandle<DescriptorHeap>
	{
		static DescriptorHeap Create(Context context, uint32_t numDescriptors);
		void Destroy();

		void WriteStorageImages(uint32_t startIndex, Span<ImageView> storageImages) const;
		void WriteSampledImages(uint32_t startIndex, Span<ImageView> sampledImages) const;
		void WriteSamplers(uint32_t startIndex, Span<Sampler> samplers) const;
	};

	struct PipelineLayout : RenderHandle<PipelineLayout>
	{
		static PipelineLayout Create(Context context, const PipelineLayoutInfo& layoutInfo);
	};

	struct Pipeline : RenderHandle<Pipeline>
	{
		static Pipeline Create(Context context, const PipelineInfo& info);
	};

	struct RayTracingPipeline : RenderHandle<RayTracingPipeline>
	{
		static RayTracingPipeline Create(Context context, const RayTracingPipelineInfo& info);
	};

	using GeometryID = UniqueID;

	struct AccelerationStructure : RenderHandle<AccelerationStructure>
	{
		static AccelerationStructure Create(Context context);

		GeometryID AddGeometry(Span<Vec3> vertexPositions, Span<uint32_t> indices);
		void AddInstance(GeometryID geometry, const Mat4& transform, uint32_t customInstanceIndex);

		uint64_t GetTopLevelAddress();
	};

	struct RenderPass : RenderHandle<RenderPass>
	{

	};

	template<>
	struct RenderHandle<RenderPass>::Impl
	{
		Function<void(RenderGraph&, int32_t)> RunFunc;
		Fence Fence;
	};

}

YUKI_ENUM_HASH(Yuki::RHI::ShaderStage);
