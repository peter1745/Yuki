#include "VulkanRHI.hpp"
#include "VulkanUtils.hpp"
#include "VulkanShaderCompiler.hpp"

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

	PipelineLayout PipelineLayout::Create(Context context, const PipelineLayoutInfo& info)
	{
		auto layout = new Impl();

		DynamicArray<VkDescriptorSetLayout> descriptorSetLayouts;
		descriptorSetLayouts.reserve(info.DescriptorLayouts.Count());
		for (auto layoutHandle : info.DescriptorLayouts)
			descriptorSetLayouts.emplace_back(layoutHandle->Handle);

		VkPushConstantRange pushConstants =
		{
			.stageFlags = VK_SHADER_STAGE_ALL,
			.offset = 0,
			.size = info.PushConstantSize,
		};

		VkPipelineLayoutCreateInfo layoutInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.pNext = nullptr,
			.setLayoutCount = Cast<uint32_t>(descriptorSetLayouts.size()),
			.pSetLayouts = descriptorSetLayouts.data(),
			.pushConstantRangeCount = info.PushConstantSize > 0 ? 1U : 0U,
			.pPushConstantRanges = info.PushConstantSize > 0 ? &pushConstants : nullptr,
		};

		YUKI_VK_CHECK(vkCreatePipelineLayout(context->Device, &layoutInfo, nullptr, &layout->Handle));

		return { layout };
	}

	Pipeline Pipeline::Create(Context context, const PipelineInfo& info)
	{
		auto pipeline = new Impl();
		pipeline->Layout = info.Layout;

		DynamicArray<VkPipelineShaderStageCreateInfo> shaderStages;

		shaderStages.reserve(info.Shaders.Count());

		for (const auto& shaderInfo : info.Shaders)
		{
			if (!std::filesystem::exists(shaderInfo.FilePath))
				continue;

			auto& stage = shaderStages.emplace_back();
			stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage.pNext = nullptr;
			stage.module = context->ShaderCompiler->CompileOrGetModule(context->Device, shaderInfo.FilePath, shaderInfo.Stage);
			stage.stage = c_ShaderStageLookup.at(shaderInfo.Stage);
			stage.pName = "main";
		}

		DynamicArray<VkFormat> colorAttachmentFormats;
		DynamicArray<VkPipelineColorBlendAttachmentState> colorAttachmentBlendStates;
		for (const auto& colorAttachmentInfo : info.ColorAttachments)
		{
			colorAttachmentFormats.emplace_back(Image::Impl::ImageFormatToVkFormat(colorAttachmentInfo.Format));

			auto& blendStateInfo = colorAttachmentBlendStates.emplace_back();
			blendStateInfo.blendEnable = VK_TRUE;
			blendStateInfo.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			blendStateInfo.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendStateInfo.colorBlendOp = VK_BLEND_OP_ADD;
			blendStateInfo.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
			blendStateInfo.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			blendStateInfo.alphaBlendOp = VK_BLEND_OP_ADD;
			blendStateInfo.colorWriteMask = 0xF;
		}

		VkPipelineRenderingCreateInfo renderingCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
			.colorAttachmentCount = uint32_t(colorAttachmentFormats.size()),
			.pColorAttachmentFormats = colorAttachmentFormats.data(),
			.depthAttachmentFormat = VK_FORMAT_UNDEFINED,
			.stencilAttachmentFormat = VK_FORMAT_UNDEFINED
		};

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
		};

		VkPipelineViewportStateCreateInfo viewportStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1,
			.scissorCount = 1
		};

		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo =
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

		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE
		};

		VkPipelineDepthStencilStateCreateInfo depthStencilCreateInfo =
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

		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.attachmentCount = uint32_t(colorAttachmentBlendStates.size()),
			.pAttachments = colorAttachmentBlendStates.data()
		};

		constexpr auto dynamicStates = std::array{ VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
			.dynamicStateCount = uint32_t(dynamicStates.size()),
			.pDynamicStates = dynamicStates.data()
		};

		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = { .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO, };
		VkGraphicsPipelineCreateInfo pipelineCreateInfo =
		{
			.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.pNext = &renderingCreateInfo,
			.stageCount = uint32_t(shaderStages.size()),
			.pStages = shaderStages.data(),
			.pVertexInputState = &vertexInputStateCreateInfo,
			.pInputAssemblyState = &inputAssemblyStateCreateInfo,
			.pViewportState = &viewportStateCreateInfo,
			.pRasterizationState = &rasterizationStateCreateInfo,
			.pMultisampleState = &multisampleStateCreateInfo,
			.pDepthStencilState = &depthStencilCreateInfo,
			.pColorBlendState = &colorBlendStateCreateInfo,
			.pDynamicState = &dynamicStateCreateInfo,
			.layout = pipeline->Layout->Handle,
		};

		YUKI_VK_CHECK(vkCreateGraphicsPipelines(context->Device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &pipeline->Handle));

		return { pipeline };
	}

	/*void VulkanRenderDevice::PipelineDestroy(PipelineRH InPipeline)
	{
		auto& pipeline = m_Pipelines[InPipeline];
		vkDestroyPipeline(m_Device, pipeline.Handle, nullptr);
		vkDestroyPipelineLayout(m_Device, pipeline.Layout, nullptr);
		m_Pipelines.Return(InPipeline);
	}*/

	RayTracingPipeline RayTracingPipeline::Create(Context context, const RayTracingPipelineInfo& info)
	{
		auto pipeline = new Impl();
		pipeline->Layout = info.Layout;

		DynamicArray<VkPipelineShaderStageCreateInfo> shaderStages;

		shaderStages.reserve(info.Shaders.Count());

		for (const auto& shaderInfo : info.Shaders)
		{
			if (!std::filesystem::exists(shaderInfo.FilePath))
				continue;

			auto& stage = shaderStages.emplace_back();
			stage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			stage.pNext = nullptr;
			stage.module = context->ShaderCompiler->CompileOrGetModule(context->Device, shaderInfo.FilePath, shaderInfo.Stage);
			stage.stage = c_ShaderStageLookup.at(shaderInfo.Stage);
			stage.pName = "main";
		}

		DynamicArray<VkRayTracingShaderGroupCreateInfoKHR> shaderGroups;
		for (uint32_t i = 0; i < shaderStages.size(); i++)
		{
			const auto& stageInfo = shaderStages[i];

			auto& group = shaderGroups.emplace_back();
			group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;

			switch (stageInfo.stage)
			{
			case VK_SHADER_STAGE_RAYGEN_BIT_KHR:
			case VK_SHADER_STAGE_MISS_BIT_KHR:
			{
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
				group.generalShader = i;
				group.closestHitShader = VK_SHADER_UNUSED_KHR;
				group.anyHitShader = VK_SHADER_UNUSED_KHR;
				group.intersectionShader = VK_SHADER_UNUSED_KHR;
				break;
			}
			case VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR:
			{
				group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
				group.generalShader = VK_SHADER_UNUSED_KHR;
				group.closestHitShader = i;
				group.anyHitShader = VK_SHADER_UNUSED_KHR;
				group.intersectionShader = VK_SHADER_UNUSED_KHR;
				break;
			}
			}
		}

		VkRayTracingPipelineCreateInfoKHR rayTracingPipelineInfo =
		{
			.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
			.pNext = nullptr,
			.stageCount = Cast<uint32_t>(shaderStages.size()),
			.pStages = shaderStages.data(),
			.groupCount = Cast<uint32_t>(shaderGroups.size()),
			.pGroups = shaderGroups.data(),
			.maxPipelineRayRecursionDepth = 1,
			.layout = pipeline->Layout->Handle,
		};
		YUKI_VK_CHECK(vkCreateRayTracingPipelinesKHR(context->Device, {}, {}, 1, &rayTracingPipelineInfo, nullptr, &pipeline->Handle));

		const auto& rtProperties = context->GetFeature<VulkanRaytracingFeature>().GetRayTracingProperties();

		uint32_t handleCount = Cast<uint32_t>(info.Shaders.Count());
		uint32_t handleSize = rtProperties.shaderGroupHandleSize;
		uint32_t handleSizeAligned = AlignUp(handleSize, rtProperties.shaderGroupHandleAlignment);

		pipeline->RayGenRegion = {
			.stride = AlignUp(handleSizeAligned, rtProperties.shaderGroupBaseAlignment),
			.size = AlignUp(handleSizeAligned, rtProperties.shaderGroupBaseAlignment),
		};

		pipeline->MissGenRegion = {
			.stride = handleSizeAligned,
			.size = AlignUp(1 * handleSizeAligned, rtProperties.shaderGroupBaseAlignment),
		};

		pipeline->ClosestHitGenRegion = {
			.stride = handleSizeAligned,
			.size = AlignUp(1 * handleSizeAligned, rtProperties.shaderGroupBaseAlignment),
		};

		uint32_t dataSize = handleCount * handleSize;
		DynamicArray<uint8_t> handles(dataSize);
		vkGetRayTracingShaderGroupHandlesKHR(context->Device, pipeline->Handle, 0, handleCount, dataSize, handles.data());

		uint64_t bufferSize = pipeline->RayGenRegion.size + pipeline->MissGenRegion.size + pipeline->ClosestHitGenRegion.size + pipeline->CallableGenRegion.size;
		pipeline->SBTBuffer = Buffer::Create(context, bufferSize, BufferUsage::ShaderBindingTable, BufferFlags::Mapped | BufferFlags::DeviceLocal);

		uint64_t bufferAddress = pipeline->SBTBuffer->Address;
		pipeline->RayGenRegion.deviceAddress = bufferAddress;
		pipeline->MissGenRegion.deviceAddress = bufferAddress + pipeline->RayGenRegion.size;
		pipeline->ClosestHitGenRegion.deviceAddress = bufferAddress + pipeline->RayGenRegion.size + pipeline->MissGenRegion.size;

		auto GetHandle = [&](uint32_t index) { return handles.data() + index * handleSize; };

		auto* bufferData = reinterpret_cast<uint8_t*>(pipeline->SBTBuffer.GetMappedMemory());
		uint8_t* dataPtr = bufferData;
		uint32_t handleIndex = 0;

		memcpy(dataPtr, GetHandle(handleIndex++), handleSize);

		dataPtr = bufferData + pipeline->RayGenRegion.size;
		for (uint32_t i = 0; i < 1; i++)
		{
			memcpy(dataPtr, GetHandle(handleIndex++), handleSize);
			dataPtr += pipeline->MissGenRegion.stride;
		}

		dataPtr = bufferData + pipeline->RayGenRegion.size + pipeline->MissGenRegion.size;
		for (uint32_t i = 0; i < 1; i++)
		{
			memcpy(dataPtr, GetHandle(handleIndex++), handleSize);
			dataPtr += pipeline->ClosestHitGenRegion.stride;
		}

		return { pipeline };
	}

	/*void VulkanRenderDevice::RayTracingPipelineDestroy(RayTracingPipelineRH InPipeline)
	{
		auto& pipeline = m_RayTracingPipelines[InPipeline];

		BufferDestroy(pipeline.SBTBuffer);

		vkDestroyPipeline(m_Device, pipeline.Handle, nullptr);
		vkDestroyPipelineLayout(m_Device, pipeline.Layout, nullptr);
		m_RayTracingPipelines.Return(InPipeline);
	}*/

}
