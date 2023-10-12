#pragma once

#include "Application.hpp"
#include "Logging.hpp"

namespace Yuki {

#define YUKI_EXIT_SUCCESS 0
#define YUKI_EXIT_FAILURE 1

	template<typename TAppClass>
	struct AppRunner
	{
		TAppClass* App = nullptr;

		bool Initialize()
		{
			Logging::Initialize();

			App = new TAppClass();
			//App->ErrorReporter = DefaultErrorReporter{};
			//return !App->ErrorReporter.HasError();
			return true;
		}

		int operator()(int argc, char* argv[])
		{
			if (!Initialize())
				return YUKI_EXIT_FAILURE;

			App->Run();

			int exitCode = YUKI_EXIT_SUCCESS;

			/*if (App->ErrorReporter.HasError())
			{
				ExitCode = EXIT_FAILURE;

				do
				{
					App->ErrorReporter.ProcessNext();
				} while (App->ErrorReporter.HasError());
			}*/

			return exitCode;
		}
	};
}

#define YUKI_DECLARE_APPLICATION(AppClass)			\
int main(int argc, char* argv[])					\
{													\
	return Yuki::AppRunner<AppClass>()(argc, argv);	\
}													\
