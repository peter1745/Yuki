#pragma once

#include "Engine/Common/Core.hpp"

#include "Mesh.hpp"

namespace Yuki {

	template<typename... TLambdas>
	struct Visitor : TLambdas... { using TLambdas::operator()...; };
	template<typename... TLambdas>
	Visitor(TLambdas...) -> Visitor<TLambdas...>;

	class glTFLoader
	{
	public:
		void Load(const std::filesystem::path& filepath, Model& model);
	};

}
