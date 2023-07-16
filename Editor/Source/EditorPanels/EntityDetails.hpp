#pragma once

#include "EditorPanel.hpp"

#include <flecs/flecs.h>
#include <string>

namespace YukiEditor {

	class EntityDetails : public EditorPanel
	{
	public:
		void Draw() override;

		void SetCurrentEntity(flecs::entity InEntity) { m_CurrentEntity = InEntity; }

	private:
		flecs::entity m_CurrentEntity = flecs::entity::null();
	};

}
