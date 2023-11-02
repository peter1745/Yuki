#include "RenderGraph.hpp"

namespace Yuki {

	/*RenderGraph::RenderGraph(RHI::Context context)
		: m_Context(context)
	{
		m_GraphicsQueue = context.RequestQueue(RHI::QueueType::Graphics);
		m_CommandPool = RHI::CommandPool::Create(context, m_GraphicsQueue);
	}

	RHI::RenderPass RenderGraph::AddPass(const RenderPassInfo& info)
	{
		auto pass = new RHI::RenderPass::Impl();
		pass->RunFunc = std::move(info.Run);
		pass->Fence = RHI::Fence::Create(m_Context);
		info.Initialize(*this, Cast<int32_t>(m_Passes.size()));
		return m_Passes.emplace_back(pass);
	}

	void RenderGraph::Execute()
	{
		m_CommandPool.Reset();

		for (size_t i = 0; i < m_Passes.size(); i++)
		{
			m_ActivePass = Cast<int32_t>(i);
			m_Passes[i]->RunFunc(*this, m_ActivePass);
		}

		for (auto pass : m_Passes)
			pass->Fence.Wait();
	}

	RHI::CommandList RenderGraph::StartPass() const
	{
		auto cmd = m_CommandPool.NewList();
		cmd.Begin();
		return cmd;
	}

	void RenderGraph::EndPass(RHI::CommandList cmd) const
	{
		cmd.End();

		if (m_ActivePass > 0)
		{
			int32_t prevPass = m_ActivePass - 1;
			m_GraphicsQueue.Submit({ cmd }, { m_Passes[prevPass]->Fence }, { m_Passes[m_ActivePass]->Fence });
		}
		else
		{
			m_GraphicsQueue.Submit({ cmd }, {}, { m_Passes[m_ActivePass]->Fence });
		}
	}*/

}
