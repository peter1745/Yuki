#include "InputContext.hpp"

namespace Yuki {

	void InputContext::BindAction(InputActionID actionID, ActionFunction func)
	{
		if (m_ActionBindings.contains(actionID))
		{
			__debugbreak();
			// Fucking error man
			return;
		}

		m_ActionBindings[actionID] = std::move(func);
	}

}
