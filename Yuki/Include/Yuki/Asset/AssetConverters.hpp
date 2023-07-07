#pragma once

namespace Yuki {

	class AssetConverter
	{
	public:
		virtual void Convert(const std::filesystem::path& InFilePath) const = 0;
	};

	class MeshConverter : public AssetConverter
	{
	public:
		void Convert(const std::filesystem::path& InFilePath) const override;
	};

}
