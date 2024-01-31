#pragma once

#include "Engine/Core/Core.hpp"
#include "Engine/Core/Handle.hpp"
#include "Engine/Core/EnumFlags.hpp"

#include <Aura/Span.hpp>

#include <filesystem>

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
		TransferDst            = 1 << 3
	};
	inline void MakeEnumFlags(ImageUsage){}

	struct Image : Handle<Image>
	{
		static Image Create(RHIContext context, uint32_t width, uint32_t height, ImageFormat format, ImageUsage usage);
		void Destroy();
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

	enum class ShaderStage
	{
		Vertex, Fragment
	};

	struct GraphicsPipelineConfig
	{
		struct ShaderConfig
		{
			ShaderStage Stage;
			std::filesystem::path FilePath;
		};

		std::vector<ShaderConfig> Shaders;
		uint32_t PushConstantSize;

		std::vector<ImageFormat> ColorAttachmentFormats;
	};

	struct GraphicsPipeline : Handle<GraphicsPipeline>
	{
		static GraphicsPipeline Create(RHIContext context, const GraphicsPipelineConfig& config);
		void Destroy();
	};

	struct RenderingAttachment
	{
		ImageView Target;
	};

	struct CommandList : Handle<CommandList>
	{
		void BeginRendering(Aura::Span<RenderingAttachment> colorAttachments) const;
		void EndRendering() const;

		void TransitionImage(Image image, ImageLayout layout) const;
	};

	struct CommandPool : Handle<CommandPool>
	{
		static CommandPool Create(RHIContext context, Queue queue);
		void Destroy();

		void Reset() const;

		CommandList NewList() const;
	};

}
