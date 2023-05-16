#pragma once

#include "../Memory/Unique.hpp"

namespace Yuki {

	struct WindowAttributes
	{
		std::string Title = "Yuki Application";
		uint32_t Width = 0;
		uint32_t Height = 0;
		bool Maximized = false;
	};

	class GenericWindow
	{
	public:
		virtual ~GenericWindow() = default;

		virtual void Create() = 0;

		virtual void ProcessEvents() = 0;

		virtual void Show() = 0;

		virtual bool ShouldClose() const = 0;

		virtual const WindowAttributes& GetAttributes() const = 0;

	public:
		static Unique<GenericWindow> New(WindowAttributes InAttributes);
	};

}
