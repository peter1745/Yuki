#pragma once

#include "Yuki/Rendering/MeshData.hpp"
#include "Yuki/Rendering/RenderContext.hpp"

namespace Yuki {

	class RenderContext;

	template<typename... TLambdas>
	struct ImageVisitor : TLambdas... { using TLambdas::operator()...; };
	template<typename... TLambdas>
	ImageVisitor(TLambdas...) -> ImageVisitor<TLambdas...>;

	class MeshLoader
	{
	public:
		static LoadedMesh LoadGLTFMesh(RenderContext* InContext, const std::filesystem::path& InFilePath);
	};

}
