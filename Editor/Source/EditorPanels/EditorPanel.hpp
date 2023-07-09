#pragma once

namespace YukiEditor {

	class EditorPanel
	{
	public:
		virtual ~EditorPanel() = default;
		
		virtual void Update(float InDeltaTime){}
		virtual void Draw() = 0;
	};

}
