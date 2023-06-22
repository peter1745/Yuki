#pragma once

#include "Yuki/IO/KeyCodes.hpp"

#include "Yuki/Memory/Unique.hpp"

#include "Yuki/EventSystem/Event.hpp"
#include "Yuki/EventSystem/WindowEvents.hpp"

#include "Yuki/Rendering/RHI/RenderContext.hpp"
#include "Yuki/Rendering/RHI/Viewport.hpp"

#include <functional>
#include <string>

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

	enum class CursorState { Normal, Locked };

	class GenericWindow
	{
	public:
		virtual ~GenericWindow() = default;

		virtual void Create() = 0;
		virtual void Destroy() = 0;

		virtual void ProcessEvents() = 0;

		virtual void Show() = 0;

		virtual const WindowAttributes& GetAttributes() const = 0;

		virtual Viewport* GetViewport() const = 0;

		virtual int64_t GetRawMouseDeltaX() = 0;
		virtual int64_t GetRawMouseDeltaY() = 0;

		virtual void SetCursorState(CursorState InState) = 0;

		virtual bool IsKeyReleased(KeyCode InKeyCode) const = 0;
		virtual bool IsKeyPressed(KeyCode InKeyCode) const = 0;

		virtual bool IsMouseButtonReleased(MouseButton InButton) const = 0;
		virtual bool IsMouseButtonPressed(MouseButton InButton) const = 0;

	public:
		static Unique<GenericWindow> New(RenderContext* InRenderContext, WindowAttributes InAttributes);
	};

}
