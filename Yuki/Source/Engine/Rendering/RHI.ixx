module;

#include <filesystem>

export module Yuki.Rendering:RHI;

import Aura;

export namespace Yuki {

	struct Queue;
	struct CommandList;
	struct Window;

	enum class QueueType { Graphics, Compute, Transfer };

	struct RHIContext : Aura::Handle<RHIContext>
	{
		static RHIContext Create();
		void Destroy();

		Queue RequestQueue(QueueType type) const;
	};

	struct Shader : Aura::Handle<Shader>
	{
	};

	struct ShaderLibrary : Aura::Handle<ShaderLibrary>
	{
		static ShaderLibrary Create(RHIContext context);
		void Destroy();

		Shader Compile(const std::filesystem::path& filepath) const;
	};

	struct Fence : Aura::Handle<Fence>
	{
		static Fence Create(RHIContext context);
		void Destroy();

		void Wait(uint64_t value = 0) const;

	};

	struct Queue : Aura::Handle<Queue>
	{
		void ExecuteCommandLists(const Aura::Span<CommandList> commandLists, const Aura::Span<Fence> signalFences) const;
	};

	struct Swapchain : Aura::Handle<Swapchain>
	{
		static Swapchain Create(RHIContext context, Queue queue, Window window);
		void Destroy();

		void Present() const;

	};

	struct GraphicsPipeline : Aura::Handle<GraphicsPipeline>
	{
		static GraphicsPipeline Create(RHIContext context);
	};

	struct CommandList : Aura::Handle<CommandList>
	{
		void End() const;
	};

	struct CommandAllocator : Aura::Handle<CommandAllocator>
	{
		static CommandAllocator Create(RHIContext context);

		CommandList NewList() const;

		void Reset() const;
	};

}
