#pragma once

namespace Yuki {

	struct WindowAttributes
	{
		std::string Title = "Yuki Application";
		uint32_t Width = 0;
		uint32_t Height = 0;
	};

	class GenericWindow
	{
	public:
		virtual ~GenericWindow() = default;

		virtual void Create() = 0;

		virtual void ProcessEvents() const = 0;

	public:
		static std::unique_ptr<GenericWindow> New(WindowAttributes InAttributes);
	};

}
