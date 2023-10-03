#pragma once

#include "MessageBase.hpp"

#include "Engine/Common/UniqueID.hpp"

namespace Yuki {

	struct WindowCloseMessage : public MessageBase
	{
		UniqueID Handle;
		bool IsPrimaryWindow;
	};

}
