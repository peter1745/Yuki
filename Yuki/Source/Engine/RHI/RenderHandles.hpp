#pragma once

#include "Engine/Common/Core.hpp"
#include "Engine/Common/UniqueID.hpp"
#include "Engine/Containers/Span.hpp"

#include "RenderFeatures.hpp"

#include <filesystem>

namespace Yuki {

	class WindowSystem;

}

namespace Yuki::RHI {

	template<typename T>
	struct RenderHandle
	{
		struct Impl;

		RenderHandle() = default;
		RenderHandle(Impl* InImpl)
			: m_Impl(InImpl) {}

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
	YUKI_RENDER_HANDLE(Pipeline);
	YUKI_RENDER_HANDLE(RayTracingPipeline);
	YUKI_RENDER_HANDLE(DescriptorSetLayout);
	YUKI_RENDER_HANDLE(DescriptorPool);
	YUKI_RENDER_HANDLE(DescriptorSet);
	YUKI_RENDER_HANDLE(Fence);
	YUKI_RENDER_HANDLE(CommandPool);
	YUKI_RENDER_HANDLE(CommandList);
	YUKI_RENDER_HANDLE(Image);
	YUKI_RENDER_HANDLE(ImageView);
	YUKI_RENDER_HANDLE(Buffer);
	YUKI_RENDER_HANDLE(AccelerationStructure);

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

	YUKI_FLAG_ENUM(ShaderStage)
	{
		None = 0,
		Vertex = 1 << 0,
		Fragment = 1 << 1,
		RayGeneration = 1 << 2,
		RayMiss = 1 << 3,
		RayClosestHit = 1 << 4,
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

	struct PipelineInfo
	{
		Span<PipelineShaderInfo> Shaders;
		uint32_t PushConstantSize = 0;
		Span<DescriptorSetLayoutRH> DescriptorLayouts;

		struct ColorAttachmentInfo { ImageFormat Format; };
		DynamicArray<ColorAttachmentInfo> ColorAttachments;
	};

	struct RayTracingPipelineInfo
	{
		Span<PipelineShaderInfo> Shaders;
		uint32_t PushConstantSize = 0;
		Span<DescriptorSetLayoutRH> DescriptorLayouts;
	};

	enum class DescriptorType
	{
		Sampler,
		CombinedImageSampler,
		SampledImage,
		StorageImage,
		UniformTexelBuffer,
		StorageTexelBuffer,
		UniformBuffer,
		StorageBuffer,
		UniformBufferDynamic,
		StorageBufferDynamic,
		InputAttachment,
	};

	struct DescriptorSetLayoutInfo
	{
		ShaderStage Stages = ShaderStage::None;
		struct DescriptorInfo { uint32_t Count; DescriptorType Type; };
		DynamicArray<DescriptorInfo> Descriptors;
	};

	struct DescriptorCount
	{
		DescriptorType Type;
		uint32_t Count;
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

		static Context Create(Config InConfig);

		bool IsFeatureEnabled(RendererFeature InFeature) const;

		Queue RequestQueue(QueueType InType) const;
	};

	struct Queue : RenderHandle<Queue>
	{
		void AcquireImages(Span<Swapchain> InSwapchains, Span<Fence> InFences);
		void Submit(Span<CommandListRH> InCommandLists, Span<FenceRH> InWaits, Span<FenceRH> InSignals);
		void Present(Span<Swapchain> InSwapchains, Span<Fence> InFences);
	};

	struct Swapchain : RenderHandle<Swapchain>
	{
		static Swapchain Create(Context InContext, const WindowSystem& InWindowSystem, UniqueID InWindowHandle);

		void Recreate() const;

		ImageRH GetCurrentImage();
		ImageViewRH GetCurrentImageView();
	};

	struct Fence : RenderHandle<Fence>
	{
		static Fence Create(Context InContext);
		void Destroy();

		void Wait(uint64_t InValue = 0);
	};

	struct CommandPool : RenderHandle<CommandPool>
	{
		static CommandPool Create(Context InContext, QueueRH InQueue);
		void Destroy();

		void Reset();
		CommandList NewList();
	};

	struct Viewport
	{
		float X;
		float Y;
		float Width;
		float Height;
	};

	struct CommandList : RenderHandle<CommandList>
	{
		friend CommandPool;

		void Begin();
		void ImageBarrier(ImageBarrier InBarrier);
		void BeginRendering(RenderTarget InRenderTarget);
		void EndRendering();
		void CopyBuffer(BufferRH InDest, BufferRH InSrc);
		void PushConstants(PipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize);
		void PushConstants(RayTracingPipelineRH InPipeline, ShaderStage InStages, const void* InData, uint32_t InDataSize);
		void BindDescriptorSets(PipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets);
		void BindDescriptorSets(RayTracingPipelineRH InPipeline, Span<DescriptorSetRH> InDescriptorSets);
		void BindPipeline(PipelineRH InPipeline);
		void BindPipeline(RayTracingPipelineRH InPipeline);
		void BindIndexBuffer(BufferRH InBuffer);
		void SetViewport(Viewport InViewport);
		void DrawIndexed(uint32_t InIndexCount, uint32_t InIndexOffset, uint32_t InInstanceIndex);
		void TraceRay(RayTracingPipelineRH InPipeline, uint32_t InWidth, uint32_t InHeight);
		void End();
	};

	struct Image : RenderHandle<Image>
	{
		friend Swapchain;
	};

	struct ImageView : RenderHandle<ImageView>
	{
		static ImageView Create(Context InContext, ImageRH InImage);
		void Destroy();
	};

	struct Buffer : RenderHandle<Buffer>
	{
		static Buffer Create(Context InContext, uint64_t InSize, BufferUsage InUsage, bool InHostAccess = false);
		void Destroy();

		void SetData(const void* InData, uint64_t InDataSize = ~0);
		uint64_t GetDeviceAddress();
		void* GetMappedMemory();
	};

	struct DescriptorSetLayout : RenderHandle<DescriptorSetLayout>
	{
		static DescriptorSetLayout Create(Context InContext, const DescriptorSetLayoutInfo& InInfo);
	};

	struct DescriptorPool : RenderHandle<DescriptorPool>
	{
		static DescriptorPool Create(Context InContext, Span<DescriptorCount> InDescriptorCounts);

		DescriptorSet AllocateDescriptorSet(DescriptorSetLayoutRH InLayout);
	};

	struct DescriptorSet : RenderHandle<DescriptorSet>
	{
		void Write(uint32_t InBinding, Span<ImageViewRH> InImageViews, uint32_t InArrayOffset);
	};

	struct Pipeline : RenderHandle<Pipeline>
	{
		static Pipeline Create(Context InContext, const PipelineInfo& InInfo);
	};

	struct RayTracingPipeline : RenderHandle<RayTracingPipeline>
	{
		static RayTracingPipeline Create(Context InContext, const RayTracingPipelineInfo& InInfo);
	};

	struct AccelerationStructure : RenderHandle<AccelerationStructure>
	{
		static AccelerationStructure Create(Context InContext, BufferRH InVertexBuffer, BufferRH InIndexBuffer);

		uint64_t GetTopLevelAddress();
	};

}

YUKI_ENUM_HASH(Yuki::RHI::ShaderStage);
