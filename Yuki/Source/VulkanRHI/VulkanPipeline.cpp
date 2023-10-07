#include "VulkanPipeline.hpp"
#include "VulkanRenderDevice.hpp"
#include "VulkanUtils.hpp"

#include "Features/VulkanRaytracingFeature.hpp"

namespace Yuki::RHI {

	static const HashMap<ShaderStage, VkShaderStageFlagBits> c_ShaderStageLookup =
	{
		{ ShaderStage::Vertex,			VK_SHADER_STAGE_VERTEX_BIT },
		{ ShaderStage::Fragment,		VK_SHADER_STAGE_FRAGMENT_BIT },
		{ ShaderStage::RayGeneration,	VK_SHADER_STAGE_RAYGEN_BIT_KHR },
		{ ShaderStage::RayMiss,			VK_SHADER_STAGE_MISS_BIT_KHR },
		{ ShaderStage::RayClosestHit,	VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR },
	};

	PipelineRH VulkanRenderDevice::PipelineCreate(const PipelineInfo& InPipelineInfo)
	{
		auto[Handle, Pipeline] = m_Pipelines.Acquire();

		DynamicArray<VkPipelineShaderStageCreateInfo> ShaderStages;

		ShaderStages.reserve(InPipelineInfo.Shaders.Count());

		for (const auto& ShaderInfo : InPipelineInfo.Shaders)
		{
			if (!std::filesystem::exists(ShaderInfo.FilePath))
				continue;

			auto& Stage = ShaderStages.emplace_back();
			Stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			Stage.pNext = nullptr;
			Stage.module = m_ShaderCompiler.CompileOrGetModule(m_Device, ShaderInfo.FilePath, ShaderInfo.Stage);
			Stage.stage = c_ShaderStageLookup.at(ShaderInfo.Stage);
			Stage.pName = "main";
		}

		DynamicArray<VkDescriptorSetLayout> DescriptorSetLayouts;
		DescriptorSetLayouts.reserve(InPipelineInfo.DescriptorLayouts.Count());
		for (auto LayoutHandle : InPipelineInfo.DescriptorLayouts)
			DescriptorSetLayouts.emplace_back(m_DescriptorSetLayouts[LayoutHandle].Handle);

		VkPushConstantRange PushConstants =
		{
			.stageFlags = VK_SHADER_STAGE_ALL,
			.offset = 0,
			.size = InPipelineInfo.PushConstantSize,
		};

		VkPipelineLayoutCreateInfo LayoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.setLayoutCount = Cast<uint32_t>(DescriptorSetLayouts.size()),
			.pSetLayouts = DescriptorSetLayouts.data(),
			.pushConstantRangeCount = InPipelineInfo.PushConstantSize > 0 ? 1U : 0U,
			.pPushConstantRanges = InPipelineInfo.PushConstantSize > 0 ? &PushConstants : nullptr,
		};

		YUKI_VERIFY(vkCreatePipelineLayout(m_Device, &LayoutInfo, nullptr, &Pipeline.Layout) == VK_SUCCESS);

		DynamicArray<VkFormat> ColorAttachmentFormats;
		DynamicArray<VkPipelineColorBlendAttachmentState> ColorAttachmentBlendStates;
		for (const auto& ColorAttachmentInfo : InPipelineInfo.ColorAttachments)
		{
			ColorAttachmentFormats.emplace_back(Vulkan::ImageFormatToVkFormat(ColorAttachmentInfo.Format));

			auto& BlendStateInfo = ColorAttachmentBlendStates.emplace_back();
			BlendStateInfo.blendEnable = VK_TRUE;
			BlendStateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			BlendStateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			BlendStateInfo.colorBlendOp = VK_BLEND_OP_ADD;
			BlendStateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			BlendStateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			BlendStateInfo.alphaBlendOp = VK_BLEND_OP_ADD;
			BlendStateInfo.colorWriteMask = 0xF;
		}

		VkPipelineRenderingCreateInfo RenderingCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = uint32_t(ColorAttachmentFormats.size()),
			.pColorAttachmentFormats = ColorAttachmentFormats.data(),
			.depthAttachmentFormat = VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		VkPipelineInputAssemblyStateCreateInfo InputAssemblyStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		};

		VkPipelineViewportStateCreateInfo ViewportStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1
		};

		VkPipelineRasterizationStateCreateInfo RasterizationStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VK_POLYGON_MODE_FILL,
			.cullMode = VK_CULL_MODE_NONE,
			.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.depthBiasConstantFactor = 0.0f,
			.depthBiasClamp = VK_FALSE,
			.depthBiasSlopeFactor = 0.0f,
			.lineWidth = 1.0f,
		};

		VkPipelineMultisampleStateCreateInfo MultisampleStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE
		};

		VkPipelineDepthStencilStateCreateInfo DepthStencilCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_FALSE,
			.depthCompareOp = VK_COMPARE_OP_GREATER,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE,
			.front = {
				.failOp = VK_STENCIL_OP_KEEP,
				.passOp = VK_STENCIL_OP_KEEP,
				.depthFailOp = VK_STENCIL_OP_KEEP,
				.compareOp = VK_COMPARE_OP_ALWAYS,
				.compareMask = 0xFF,
				.writeMask = 0xFF,
				.reference = 0,
			},
			.back = {
				.failOp = VK_STENCIL_OP_KEEP,
				.passOp = VK_STENCIL_OP_KEEP,
				.depthFailOp = VK_STENCIL_OP_KEEP,
				.compareOp = VK_COMPARE_OP_ALWAYS,
				.compareMask = 0xFF,
				.writeMask = 0xFF,
				.reference = 0,
			},
			.minDepthBounds = 0.0f,
			.maxDepthBounds = 0.0f,
		};

		VkPipelineColorBlendStateCreateInfo ColorBlendStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = uint32_t(ColorAttachmentBlendStates.size()),
			.pAttachments = ColorAttachmentBlendStates.data()
		};

		constexpr auto DynamicStates = std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo DynamicStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = uint32_t(DynamicStates.size()),
			.pDynamicStates = DynamicStates.data()
		};

		VkPipelineVertexInputStateCreateInfo VertexInputStateCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, };
		VkGraphicsPipelineCreateInfo PipelineCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &RenderingCreateInfo,
			.stageCount = uint32_t(ShaderStages.size()),
			.pStages = ShaderStages.data(),
			.pVertexInputState = &VertexInputStateCreateInfo,
			.pInputAssemblyState = &InputAssemblyStateCreateInfo,
			.pViewportState = &ViewportStateCreateInfo,
			.pRasterizationState = &RasterizationStateCreateInfo,
			.pMultisampleState = &MultisampleStateCreateInfo,
			.pDepthStencilState = &DepthStencilCreateInfo,
			.pColorBlendState = &ColorBlendStateCreateInfo,
			.pDynamicState = &DynamicStateCreateInfo,
			.layout = Pipeline.Layout,
		};

		YUKI_VERIFY(vkCreateGraphicsPipelines(m_Device, VK_NULL_HANDLE, 1, &PipelineCreateInfo, nullptr, &Pipeline.Handle) == VK_SUCCESS);

		return Handle;
	}

	void VulkanRenderDevice::PipelineDestroy(PipelineRH InPipeline)
	{
		auto& Pipeline = m_Pipelines[InPipeline];
		vkDestroyPipeline(m_Device, Pipeline.Handle, nullptr);
		vkDestroyPipelineLayout(m_Device, Pipeline.Layout, nullptr);
		m_Pipelines.Return(InPipeline);
	}

	RayTracingPipelineRH VulkanRenderDevice::RayTracingPipelineCreate(const RayTracingPipelineInfo& InPipelineInfo)
	{
		auto [Handle, Pipeline] = m_RayTracingPipelines.Acquire();

		DynamicArray<VkPipelineShaderStageCreateInfo> ShaderStages;

		ShaderStages.reserve(InPipelineInfo.Shaders.Count());

		for (const auto& ShaderInfo : InPipelineInfo.Shaders)
		{
			YUKI_VERIFY(std::filesystem::exists(ShaderInfo.FilePath));

			// TODO(Peter): Multiple Miss / Hit Shaders per pipeline

			auto& Stage = ShaderStages.emplace_back();
			Stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			Stage.pNext = nullptr;
			Stage.module = m_ShaderCompiler.CompileOrGetModule(m_Device, ShaderInfo.FilePath, ShaderInfo.Stage);
			Stage.stage = c_ShaderStageLookup.at(ShaderInfo.Stage);
			Stage.pName = "main";
		}

		DynamicArray<VkDescriptorSetLayout> DescriptorSetLayouts;
		DescriptorSetLayouts.reserve(InPipelineInfo.DescriptorLayouts.Count());
		for (auto LayoutHandle : InPipelineInfo.DescriptorLayouts)
			DescriptorSetLayouts.emplace_back(m_DescriptorSetLayouts[LayoutHandle].Handle);

		VkPushConstantRange PushConstants =
		{
			.stageFlags = VK_SHADER_STAGE_ALL,
			.offset = 0,
			.size = InPipelineInfo.PushConstantSize,
		};

		VkPipelineLayoutCreateInfo LayoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.setLayoutCount = Cast<uint32_t>(DescriptorSetLayouts.size()),
			.pSetLayouts = DescriptorSetLayouts.data(),
			.pushConstantRangeCount = InPipelineInfo.PushConstantSize > 0 ? 1U : 0U,
			.pPushConstantRanges = InPipelineInfo.PushConstantSize > 0 ? &PushConstants : nullptr,
		};

		YUKI_VERIFY(vkCreatePipelineLayout(m_Device, &LayoutInfo, nullptr, &Pipeline.Layout) == VK_SUCCESS);

		DynamicArray<VkRayTracingShaderGroupCreateInfoKHR> ShaderGroups;
		for (uint32_t Index = 0; Index < ShaderStages.size(); Index++)
		{
			const auto& StageInfo = ShaderStages[Index];

			auto& Group = ShaderGroups.emplace_back();
			Group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;

			switch (StageInfo.stage)
			{
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
			case VK_SHADER_STAGE_MISS_BIT_KHR:
			{
				Group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				Group.generalShader = Index;
				Group.closestHitShader = VK_SHADER_UNUSED_KHR;
				Group.anyHitShader = VK_SHADER_UNUSED_KHR;
				Group.intersectionShader = VK_SHADER_UNUSED_KHR;
				break;
			}
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
			{
				Group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				Group.generalShader = VK_SHADER_UNUSED_KHR;
				Group.closestHitShader = Index;
				Group.anyHitShader = VK_SHADER_UNUSED_KHR;
				Group.intersectionShader = VK_SHADER_UNUSED_KHR;
				break;
			}
			}
		}

		VkRayTracingPipelineCreateInfoKHR RayTracingPipelineInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.stageCount = Cast<uint32_t>(ShaderStages.size()),
			.pStages = ShaderStages.data(),
			.groupCount = Cast<uint32_t>(ShaderGroups.size()),
			.pGroups = ShaderGroups.data(),
			.maxPipelineRayRecursionDepth = 1,
			.layout = Pipeline.Layout,
		};
		YUKI_VERIFY(vkCreateRayTracingPipelinesKHR(m_Device, {}, {}, 1, &RayTracingPipelineInfo, nullptr, &Pipeline.Handle) == VK_SUCCESS);

		const auto& RTProperties = GetFeature<VulkanRaytracingFeature>().GetRayTracingProperties();

		uint32_t HandleCount = Cast<uint32_t>(InPipelineInfo.Shaders.Count());
		uint32_t HandleSize = RTProperties.shaderGroupHandleSize;
		uint32_t HandleSizeAligned = AlignUp(HandleSize, RTProperties.shaderGroupHandleAlignment);

		Pipeline.RayGenRegion = {
			.stride = AlignUp(HandleSizeAligned, RTProperties.shaderGroupBaseAlignment),
			.size = AlignUp(HandleSizeAligned, RTProperties.shaderGroupBaseAlignment),
		};

		Pipeline.MissGenRegion = {
			.stride = HandleSizeAligned,
			.size = AlignUp(1 * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment),
		};

		Pipeline.ClosestHitGenRegion = {
			.stride = HandleSizeAligned,
			.size = AlignUp(1 * HandleSizeAligned, RTProperties.shaderGroupBaseAlignment),
		};

		uint32_t DataSize = HandleCount * HandleSize;
		DynamicArray<uint8_t> Handles(DataSize);
		vkGetRayTracingShaderGroupHandlesKHR(m_Device, Pipeline.Handle, 0, HandleCount, DataSize, Handles.data());

		uint64_t BufferSize = Pipeline.RayGenRegion.size + Pipeline.MissGenRegion.size + Pipeline.ClosestHitGenRegion.size + Pipeline.CallableGenRegion.size;
		Pipeline.SBTBuffer = BufferCreate(BufferSize, BufferUsage::ShaderBindingTable, true);

		uint64_t BufferAddress = BufferGetDeviceAddress(Pipeline.SBTBuffer);
		Pipeline.RayGenRegion.deviceAddress = BufferAddress;
		Pipeline.MissGenRegion.deviceAddress = BufferAddress + Pipeline.RayGenRegion.size;
		Pipeline.ClosestHitGenRegion.deviceAddress = BufferAddress + Pipeline.RayGenRegion.size + Pipeline.MissGenRegion.size;

		auto GetHandle = [&](uint32_t Index) { return Handles.data() + Index * HandleSize; };

		auto* BufferData = reinterpret_cast<uint8_t*>(BufferGetMappedMemory(Pipeline.SBTBuffer));
		uint8_t* DataPtr = BufferData;
		uint32_t HandleIndex = 0;

		memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);

		DataPtr = BufferData + Pipeline.RayGenRegion.size;
		for (uint32_t Index = 0; Index < 1; Index++)
		{
			memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);
			DataPtr += Pipeline.MissGenRegion.stride;
		}

		DataPtr = BufferData + Pipeline.RayGenRegion.size + Pipeline.MissGenRegion.size;
		for (uint32_t Index = 0; Index < 1; Index++)
		{
			memcpy(DataPtr, GetHandle(HandleIndex++), HandleSize);
			DataPtr += Pipeline.ClosestHitGenRegion.stride;
		}

		return Handle;
	}

	void VulkanRenderDevice::RayTracingPipelineDestroy(RayTracingPipelineRH InPipeline)
	{
		auto& Pipeline = m_RayTracingPipelines[InPipeline];

		BufferDestroy(Pipeline.SBTBuffer);

		vkDestroyPipeline(m_Device, Pipeline.Handle, nullptr);
		vkDestroyPipelineLayout(m_Device, Pipeline.Layout, nullptr);
		m_RayTracingPipelines.Return(InPipeline);
	}

}
