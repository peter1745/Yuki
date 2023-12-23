#pragma once

#include <exception>
#include <stacktrace>
#include <string_view>

namespace Yuki {

	class Exception final : public std::exception
	{
	public:
		Exception(std::string_view message, std::stacktrace trace = std::stacktrace::current())
			: m_Message(message), m_Stacktrace(trace)
		{
		}

		const char* what() const noexcept override { return m_Message.data(); }
		const std::stacktrace& StackTrace() const { return m_Stacktrace; }

	private:
		std::string_view m_Message;
		const std::stacktrace m_Stacktrace;
	};

}
