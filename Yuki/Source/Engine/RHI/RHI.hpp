#pragma once

#include "Engine/Core/Core.hpp"
#include "Engine/Core/Handle.hpp"
#include "Engine/Core/EnumFlags.hpp"
#include "Engine/Containers/Span.hpp"

#include <filesystem>

namespace Yuki {

	enum class CommandListType
	{
		Graphics,
		Compute,
		Transfer
	};

	struct Queue;

	enum class RHIDeviceFeature
	{
		HostImageCopy
	};

	struct RHIContext : Handle<RHIContext>
	{
		static RHIContext Create();
		void Destroy();

		bool HasFeature(RHIDeviceFeature feature) const;

		Queue RequestQueue(CommandListType commandListType) const;
	};

	struct CommandList;
	struct Queue : Handle<Queue>
	{
		void SubmitCommandLists(Span<CommandList> commandLists) const;
	};

	class Window;

	struct Swapchain : Handle<Swapchain>
	{
		static Swapchain Create(RHIContext context, Queue queue, Window window);
		void Destroy();

		void Present() const;
	};

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
	};

}
