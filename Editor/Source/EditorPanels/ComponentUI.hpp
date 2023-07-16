#pragma once

#include <flecs/flecs.h>

namespace YukiEditor {

	template<typename TComponent>
	struct ComponentUI
	{
		static void Draw(flecs::entity InEntity, TComponent* InComponent);
	};

}
