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

		Aura::Span<Queue> RequestQueue(QueueType type, uint32_t count) const;
	};

	struct CommandList;
	struct Queue : Handle<Queue>
	{
		void SubmitCommandLists(Aura::Span<CommandList> commandLists) const;
	};

	class Window;

	struct Swapchain : Handle<Swapchain>
	{
		static Swapchain Create(RHIContext context, Window window);
		void Destroy();

		void Present() const;
	};

	/*
	enum class ShaderType
	{
		Vertex, Pixel,
		RayAnyHit, RayCallable, RayClosestHit, RayIntersection, RayMiss, RayGeneration
	};

	struct Shader : Handle<Shader>
	{
		static Shader Create(const std::filesystem::path& filepath, ShaderType type);
		void Destroy();
	};

	struct RaytracingPipeline : Handle<RaytracingPipeline>
	{
		static RaytracingPipeline Create(RHIContext context, Shader shader);
	};

	enum class BufferUsage
	{

	};
	//void MakeEnumFlags(BufferUsage) {}

	struct Buffer : Handle<Buffer>
	{
		static Buffer Create(RHIContext context, BufferUsage usage, uint32_t size);
	};

	struct CommandList : Handle<CommandList>
	{
		void BindPipeline(RaytracingPipeline pipeline) const;

		void DispatchRays(uint32_t width, uint32_t height) const;
	};

	struct CommandAllocator : Handle<CommandAllocator>
	{
		static CommandAllocator Create(RHIContext context, CommandListType commandListType);
		void Destroy();

		CommandList NewList() const;

		void Reset() const;
	};*/

}
