#pragma once

#include "Engine/Messages/WindowMessages.hpp"

#include <unordered_map>
#include <ranges>

namespace Yuki {

	class Application
	{
	public:
		bool ShouldClose = false;

	private:
		void Run();

		virtual void Update() {}

	private:
		template<typename>
		friend struct AppRunner;
	};

}
