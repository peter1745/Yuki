#include "ComponentUI.hpp"

#include <Yuki/Core/Core.hpp>
#include <Yuki/Math/Math.hpp>
#include <Yuki/World/Components/TransformComponents.hpp>
#include <Yuki/World/Components/SpaceComponents.hpp>

#include <imgui/imgui.h>

namespace YukiEditor {

	using namespace Yuki::Components;

	void ComponentUI<Translation>::Draw(flecs::entity InEntity, Translation* InTranslation)
	{
		InEntity.get_mut<GPUTransform>()->IsDirty |= ImGui::DragFloat3("Translation", &InTranslation->Value[0]);
	}

	void ComponentUI<Rotation>::Draw(flecs::entity InEntity, Rotation* InRotation)
	{
		static Yuki::Map<Rotation*, Yuki::Math::Vec3> EulerCache;

		auto& eulerAngles = EulerCache[InRotation];

		if (ImGui::DragFloat3("Rotation", &eulerAngles[0]))
		{
			Yuki::Math::Vec3 radians;
			radians.X = Yuki::Math::Radians(eulerAngles.X);
			radians.Y = Yuki::Math::Radians(eulerAngles.Y);
			radians.Z = Yuki::Math::Radians(eulerAngles.Z);

			InRotation->Value = Yuki::Math::Quat(radians);

			InEntity.get_mut<GPUTransform>()->IsDirty = true;
		}
	}

	void ComponentUI<Scale>::Draw(flecs::entity InEntity, Scale* InScale)
	{
		InEntity.get_mut<GPUTransform>()->IsDirty |= ImGui::DragFloat3("Scale", &InScale->Value[0]);
	}

	void ComponentUI<StarGenerator>::Draw(flecs::entity InEntity, StarGenerator* InGenerator)
	{
		if (ImGui::TreeNode("Mesh Settings"))
		{
			InGenerator->Regenerate |= ImGui::DragScalar("Subdivsions", ImGuiDataType_U32, &InGenerator->MeshSubdivisions);
			InGenerator->Regenerate |= ImGui::DragFloat("UV Multiplier", &InGenerator->UVMultiplier);
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Noise Settings"))
		{
			InGenerator->Regenerate |= ImGui::DragFloat("Frequency", &InGenerator->NoiseFrequency);
			InGenerator->Regenerate |= ImGui::Checkbox("Random Seed", &InGenerator->RandomSeed);

			if (!InGenerator->RandomSeed)
				InGenerator->Regenerate |= ImGui::DragInt("Seed", &InGenerator->Seed);

			ImGui::TreePop();
		}
	}

}
