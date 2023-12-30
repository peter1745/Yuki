#pragma once

namespace Yuki {

	enum class KeyCode
	{
		// Keyboard Codes (Currently maps directly to Windows Virtual Key Codes)

		Backspace = 0x08,
		Tab = 0x09,

		Enter = 0x0D,

		Shift = 0x10,
		LeftShift = 0xA0,
		RightShift = 0xA1,

		Ctrl = 0x11,
		LeftCtrl = 0xA2,
		RightCtrl = 0xA3,

		Alt = 0x12,
		LeftAlt = 0xA4,
		RightAlt = 0xA5,

		CapsLock = 0x14,
		Escape = 0x1B,
		Space = 0x20,

		PageUp = 0x21,
		PageDown = 0x22,
		End = 0x23,
		Home = 0x24,

		LeftArrow = 0x25,
		UpArrow = 0x26,
		RightArrow = 0x27,
		DownArrow = 0x28,

		Delete = 0x2E,

		Plus = 0xBB,
		Comma = 0xBC,
		Minus = 0xBD,
		Period = 0xBE,

		Num0 = 0x30,
		Num1,
		Num2,
		Num3,
		Num4,
		Num5,
		Num6,
		Num7,
		Num8,
		Num9,

		Numpad0 = 0x60,
		Numpad1,
		Numpad2,
		Numpad3,
		Numpad4,
		Numpad5,
		Numpad6,
		Numpad7,
		Numpad8,
		Numpad9,
		NumpadMultiply,
		NumpadAdd,
		NumpadSeparator,
		NumpadSubtract,
		NumpadPeriod,
		NumpadDivide,

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

		F1 = 0x70,
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
	};

	enum class GamepadInput
	{
		LeftThumbstickForward,
		LeftThumbstickRight,
		RightThumbstickForward,
		RightThumbstickRight,
		LeftTrigger,
		RightTrigger,

		ButtonA,
		ButtonB,
		ButtonX,
		ButtonY,

		LeftBumper,
		RightBumper,

		ViewButton,
		MenuButton,

		LeftThumbstickButton,
		RightThumbstickButton,
	};

}
