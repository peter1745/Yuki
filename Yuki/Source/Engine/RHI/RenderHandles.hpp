#pragma once

#include "Engine/Common/Core.hpp"

namespace Yuki::RHI {

#define YUKI_RENDER_HANDLE(Type) enum class Type##RH{}

	YUKI_RENDER_HANDLE(Queue);
	YUKI_FLAG_ENUM(QueueType)
	{
		Graphics = 1 << 0,
		Compute  = 1 << 1,
		Transfer = 1 << 2
	};

	YUKI_RENDER_HANDLE(Swapchain);
	YUKI_RENDER_HANDLE(Fence);
	YUKI_RENDER_HANDLE(CommandPool);
	YUKI_RENDER_HANDLE(CommandList);

	YUKI_RENDER_HANDLE(Image);
	YUKI_RENDER_HANDLE(ImageView);
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

	YUKI_RENDER_HANDLE(Buffer);
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

	YUKI_RENDER_HANDLE(Pipeline);
	YUKI_RENDER_HANDLE(RayTracingPipeline);
	YUKI_FLAG_ENUM(ShaderStage)
	{
		None			= 0,
		Vertex			= 1 << 0,
		Fragment		= 1 << 1,
		RayGeneration	= 1 << 2,
		RayMiss			= 1 << 3,
		RayClosestHit	= 1 << 4,
	};

	YUKI_RENDER_HANDLE(DescriptorSetLayout);
	YUKI_RENDER_HANDLE(DescriptorPool);
	YUKI_RENDER_HANDLE(DescriptorSet);
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

	YUKI_RENDER_HANDLE(AccelerationStructure);

#undef YUKI_RENDER_HANDLE
}
