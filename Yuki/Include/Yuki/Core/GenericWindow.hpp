#pragma once

#include "../Memory/Unique.hpp"
#include "../EventSystem/Event.hpp"
#include "../EventSystem/WindowEvents.hpp"

namespace Yuki {

	using WindowEventFn = std::function<void(Event*)>;

	struct WindowAttributes
	{
		std::string Title = "Yuki Application";
		uint32_t Width = 0;
		uint32_t Height = 0;
		bool Maximized = false;

		WindowEventFn EventCallback = nullptr;
	};

	class GenericWindow
	{
	public:
		virtual ~GenericWindow() = default;

		virtual void Create() = 0;

		virtual void ProcessEvents() const = 0;

		virtual void Show() = 0;

		virtual const WindowAttributes& GetAttributes() const = 0;

	public:
		static Unique<GenericWindow> New(WindowAttributes InAttributes);
	};

}
