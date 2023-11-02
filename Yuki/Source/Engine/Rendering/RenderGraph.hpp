#pragma once

#include "Engine/RHI/RenderHandles.hpp"
#include "Engine/Containers/Span.hpp"

namespace Yuki {

	/*class RenderGraph;

	struct RenderPassInfo
	{
		std::function<void(RenderGraph&, int32_t)> Initialize;
		std::function<void(RenderGraph&, int32_t)> Run;
	};

	class RenderGraph
	{
	public:
		RenderGraph(RHI::Context context);

		RHI::RenderPass AddPass(const RenderPassInfo& info);

		template<typename T>
		void SetPassData(int32_t passID, const T& data)
		{
			if (passID >= m_PassStorage.size())
				m_PassStorage.resize(passID + 1);

			m_PassStorage[passID].assign_range(std::as_bytes(std::span(&data, 1)));
		}

		template<typename T>
		T& GetPassData(int32_t passID)
		{
			return *reinterpret_cast<T*>(m_PassStorage[passID].data());
		}

		void Execute();

	public:
		RHI::CommandList StartPass() const;
		void EndPass(RHI::CommandList cmd) const;

		template<typename T>
		void Output(const T& value)
		{
			m_OutputStorage.assign_range(std::as_bytes(std::span(&value, 1)));
		}

		template<typename T>
		const T& Input() const
		{
			return *reinterpret_cast<const T*>(m_OutputStorage.data());
		}

	private:
		RHI::Context m_Context;
		RHI::Queue m_GraphicsQueue{};
		RHI::CommandPool m_CommandPool{};

		DynamicArray<RHI::RenderPass> m_Passes;
		DynamicArray<DynamicArray<std::byte>> m_PassStorage;
		DynamicArray<std::byte> m_OutputStorage;
		int32_t m_ActivePass = -1;
	};*/

}
