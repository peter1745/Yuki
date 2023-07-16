#include "EntityDetails.hpp"
#include "ComponentUI.hpp"

#include <Yuki/Core/Debug.hpp>
#include <Yuki/Core/ScopeExitGuard.hpp>
#include <Yuki/World/ComponentGroup.hpp>
#include <Yuki/World/Components/CoreComponents.hpp>
#include <Yuki/World/Components/TransformComponents.hpp>

#include <imgui/imgui.h>
#include <imgui/misc/cpp/imgui_stdlib.h>

namespace YukiEditor {

	using EditableComponents = Yuki::ComponentGroup<
		Yuki::Components::Translation,
		Yuki::Components::Rotation,
		Yuki::Components::Scale
	>;

	void EntityDetails::Draw()
	{
		YUKI_SCOPE_EXIT_GUARD(){ ImGui::End(); };

		if (!ImGui::Begin("Entity Data"))
			return;

		if (m_CurrentEntity == flecs::entity::null())
			return;

		auto* name = m_CurrentEntity.get_mut<Yuki::Components::Name>();
		ImGui::InputText("##EntityName", &name->Value);

		Yuki::ComponentIter::Each(m_CurrentEntity, EditableComponents{}, [this]<typename TComp>(TComp* InComponent)
		{
			ComponentUI<TComp>::Draw(m_CurrentEntity, InComponent);
		});
	}

}
