#include "ImageProcessor.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Yuki {

	Image ImageProcessor::CreateFromFile(RHIContext context, const std::filesystem::path& filepath) const
	{
		if (!std::filesystem::exists(filepath))
		{
			WriteLine("Can't load image {}, failed to find the file.", filepath.string());
			return {};
		}

		auto filepathStr = filepath.string();

		int32_t width, height;
		stbi_uc* data = stbi_load(filepathStr.c_str(), &width, &height, nullptr, STBI_rgb_alpha);

		YukiAssert(width > 0 && height > 0);

		auto image = Image::Create(context, {
			.Width = static_cast<uint32_t>(width),
			.Height = static_cast<uint32_t>(height),
			.Format = ImageFormat::RGBA8Unorm,
			.Usage = ImageUsage::Sampled | ImageUsage::TransferDst,
			.CreateDefaultView = true
		});

		auto uploadFence = Fence::Create(context);

		auto stagingBuffer = Buffer::Create(context, width * height * 4, BufferUsage::TransferSrc | BufferUsage::Mapped);
		stagingBuffer.SetData(reinterpret_cast<std::byte*>(data), 0, width * height * 4);

		auto queue = context.RequestQueue(QueueType::Transfer);
		auto pool = CommandPool::Create(context, queue);
		auto cmd = pool.NewList();
		cmd.TransitionImage(image, ImageLayout::TransferDst);
		cmd.CopyBufferToImage(image, stagingBuffer, width * height * 4);
		cmd.TransitionImage(image, ImageLayout::ShaderReadOnlyOptimal);

		queue.SubmitCommandLists({ cmd }, {}, { uploadFence });

		uploadFence.Wait();
		
		return image;
	}

}
