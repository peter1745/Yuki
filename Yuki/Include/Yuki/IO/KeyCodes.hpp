#pragma once

namespace Yuki {

	enum class MouseButtonState
	{
		None = -1, Pressed, Released
	};

	enum class MouseButton
	{
		None = -1,
		Left, Right, Middle
	};

	enum class KeyState
	{
		None = -1, Pressed, Released
	};

	enum class KeyCode
	{
		Unknown = -1,

		Apostrophe = 39,
		Comma = 44,
		Minus = 45,
		Period = 46,
		Slash = 47,

		Num0 = 48,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,

		Numpad0,
		Numpad1,
		Numpad2,
		Numpad3,
		Numpad4,
		Numpad5,
		Numpad6,
		Numpad7,
		Numpad8,
		Numpad9,
		NumpadEnter,
		NumpadAdd,
		NumpadMultiply,
		NumpadDivide,
		NumpadDecimal,
		NumpadSubtract,
		NumpadEqual,

		A = 65,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,

		Backspace,
		Escape,
		Space,
		Enter,
		Tab,
		CapsLock,

		Insert,
		Delete,
		Home,
		End,
		PageUp,
		PageDown,

		UpArrow,
		DownArrow,
		LeftArrow,
		RightArrow,

		LeftShift,
		RightShift,
		LeftControl,
		RightControl,
		LeftAlt,
		RightAlt

	};

}
